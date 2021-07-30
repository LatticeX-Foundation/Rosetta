// ==============================================================================
// Copyright 2020 The LatticeX Foundation
// This file is part of the Rosetta library.
//
// The Rosetta library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Rosetta library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
// ==============================================================================
#include "tensorflow/core/common_runtime/metrics.h"
#include "tensorflow/core/framework/dataset.h"
#include "tensorflow/core/framework/partial_tensor_shape.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/lib/io/buffered_inputstream.h"
#include "tensorflow/core/lib/io/inputbuffer.h"
#include "tensorflow/core/lib/io/random_inputstream.h"
#include "tensorflow/core/lib/io/record_reader.h"
#include "tensorflow/core/lib/io/zlib_compression_options.h"
#include "tensorflow/core/lib/io/zlib_inputstream.h"
#include "cc/tf/secureops/secure_base_kernel.h"
#include "cc/modules/protocol/public/include/protocol_manager.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"
#include "cc/modules/common/include/utils/rtt_logger.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include "cc/modules/common/include/utils/file_directory.h"

using namespace rosetta;

namespace tensorflow {
namespace data {
namespace {

// See documentation in ../../ops/dataset_ops.cc for a high-level
// description of the following ops.


constexpr char kPrivateTextLineDatasetName[] = "PrivateTextLine";

// PrivateTextLineDatasetOp is mostly copy from tensorflow.data.TextLineDatasetOp with subtle
// change for reading line and setting up file stream
// the main purpose is to support secure cvs file reading
class PrivateTextLineDatasetOp : public DatasetOpKernel {
 public:
  using DatasetOpKernel::DatasetOpKernel;
  PrivateTextLineDatasetOp(OpKernelConstruction* ctx) : DatasetOpKernel(ctx) {
    const NodeDef& def = ctx->def();
    unique_op_name_ = def.name();

    auto func_def = ctx->function_library()->GetFunctionLibraryDefinition();
    if (func_def) {
        std::vector<string> func_name_lists = func_def->ListFunctionNames();
        assert(func_name_lists.size() == 1);
        unique_op_name_ = func_name_lists[0] + "/" + def.name();
        log_debug << "PrivateTextLineDataset op unique name_:" << unique_op_name_;
    }
  }

  void MakeDataset(OpKernelContext* ctx, DatasetBase** output) override {
    const Tensor* filenames_tensor;
    OP_REQUIRES_OK(ctx, ctx->input("filenames", &filenames_tensor));
    OP_REQUIRES(
        ctx, filenames_tensor->dims() <= 1,
        errors::InvalidArgument("`filenames` must be a scalar or a vector."));

    string compression_type;
    OP_REQUIRES_OK(ctx, ParseScalarArgument<string>(ctx, "compression_type",
                                                    &compression_type));

    int64 buffer_size = -1;
    OP_REQUIRES_OK(
        ctx, ParseScalarArgument<int64>(ctx, "buffer_size", &buffer_size));
    OP_REQUIRES(
        ctx, buffer_size >= 0,
        errors::InvalidArgument("`buffer_size` must be >= 0 (0 == default)"));

    io::ZlibCompressionOptions zlib_compression_options =
        io::ZlibCompressionOptions::DEFAULT();
    if (compression_type == "ZLIB") {
      zlib_compression_options = io::ZlibCompressionOptions::DEFAULT();
    } else if (compression_type == "GZIP") {
      zlib_compression_options = io::ZlibCompressionOptions::GZIP();
    } else {
      OP_REQUIRES(ctx, compression_type.empty(),
                  errors::InvalidArgument("Unsupported compression_type."));
    }

    if (buffer_size != 0) {
      // Set the override size.
      zlib_compression_options.input_buffer_size = buffer_size;
    }

    string data_owner = "";
    OP_REQUIRES_OK(
        ctx, ParseScalarArgument<string>(ctx, "data_owner", &data_owner));
    string task_id = ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation());
    shared_ptr<NET_IO> netio = ProtocolManager::Instance()->GetProtocol(task_id)->GetNetHandler();
    const vector<string>& party2node = netio->GetParty2Node();
    const vector<string>& result_nodes = netio->GetResultNodes();
    vector<string> nodes = decode_reveal_nodes(data_owner, party2node, result_nodes);
    OP_REQUIRES(ctx, nodes.size() == 1, errors::InvalidArgument("Unsupported node."));
    data_owner = nodes[0];
    //OP_REQUIRES(
    //    ctx, data_owner >= 0 && data_owner <= 2,
    //    errors::InvalidArgument("`data_owner` in {0,1,2}"));

    std::vector<string> filenames;
    filenames.reserve(filenames_tensor->NumElements());
    for (int i = 0; i < filenames_tensor->NumElements(); ++i) {
      filenames.push_back(filenames_tensor->flat<string>()(i));
    }

    *output = new Dataset(ctx, std::move(filenames), compression_type,
                          zlib_compression_options, task_id, data_owner, unique_op_name_);
  }

 private:
  class Dataset : public DatasetBase {
   public:
    Dataset(OpKernelContext* ctx, std::vector<string> filenames,
            const string& compression_type,
            const io::ZlibCompressionOptions& options, const string& task_id, const string& data_owner, string unique_op_name)
        : DatasetBase(DatasetContext(ctx)),
          filenames_(std::move(filenames)),
          compression_type_(compression_type),
          use_compression_(!compression_type.empty()),
          task_id_(task_id),
          data_owner_(data_owner), 
          unique_op_name_(unique_op_name),
          options_(options) {}

    std::unique_ptr<IteratorBase> MakeIteratorInternal(
        const string& prefix) const override {
      return absl::make_unique<Iterator>(Iterator::Params{
          this, strings::StrCat(prefix, "::", kPrivateTextLineDatasetName)}, task_id_, data_owner_);
    }

    const DataTypeVector& output_dtypes() const override {
      static DataTypeVector* dtypes = new DataTypeVector({DT_STRING});
      return *dtypes;
    }

    const std::vector<PartialTensorShape>& output_shapes() const override {
      static std::vector<PartialTensorShape>* shapes =
          new std::vector<PartialTensorShape>({{}});
      return *shapes;
    }

    string DebugString() const override { return "PrivateTextLineDatasetOp::Dataset"; }

   protected:
    Status AsGraphDefInternal(SerializationContext* ctx,
                              DatasetGraphDefBuilder* b,
                              Node** output) const override {
      Node* filenames = nullptr;
      Node* compression_type = nullptr;
      Node* buffer_size = nullptr;
      TF_RETURN_IF_ERROR(b->AddVector(filenames_, &filenames));
      TF_RETURN_IF_ERROR(b->AddScalar(compression_type_, &compression_type));
      TF_RETURN_IF_ERROR(
          b->AddScalar(options_.input_buffer_size, &buffer_size));
      TF_RETURN_IF_ERROR(b->AddDataset(
          this, {filenames, compression_type, buffer_size}, output));
      return Status::OK();
    }

   private:
    class Iterator : public DatasetIterator<Dataset> {
     protected:
      struct DataFileInfo {
        int fields;
        int lines;
        char delim;
        bool with_header;
        DataFileInfo() : fields(0), lines(0), delim(','), with_header(false) {}
      };

     public:
      explicit Iterator(const Params& params, const string& task_id, const string& data_owner)
          : task_id_(task_id), data_owner_(data_owner), DatasetIterator<Dataset>(params) {
        net_io_ = ProtocolManager::Instance()->GetProtocol(task_id_)->GetNetHandler();
      }

      Status GetNextInternal(IteratorContext* ctx,
                             std::vector<Tensor>* out_tensors,
                             bool* end_of_sequence) override {
        mutex_lock l(mu_);
        do {
          // We are currently processing a file, so try to read the next line.
          // TODO: [kelvin] use fake line with zeros
          // kelvin will custom buffer_input_stream and setup the stream in the beginning
          if (is_setup_) {
            string line_contents;
            // Status s = buffered_input_stream_->ReadLine(&line_contents);
            Status s = ReadNextLine(line_contents);
            
            if (s.ok()) {
              // Produce the line as output.
              metrics::RecordTFDataBytesRead(kPrivateTextLineDatasetName,
                                             line_contents.size());
              out_tensors->emplace_back(ctx->allocator({}), DT_STRING,
                                        TensorShape({}));
              out_tensors->back().scalar<string>()() = std::move(line_contents);
              *end_of_sequence = false;
              return Status::OK();
            } else if (!errors::IsOutOfRange(s)) {
              // Report non-EOF errors to the caller.
              log_error << "got non-EOF error !";
              return s;
            }
            // We have reached the end of the current file, so maybe
            // move on to next file.
            ResetStreamsLocked();
            ++current_file_index_;
          }

          // Iteration ends when there are no more files to process.
          if (current_file_index_ == dataset()->filenames_.size()) {
            *end_of_sequence = true;
            return Status::OK();
          }

          TF_RETURN_IF_ERROR(SetupStreamsLocked(ctx->env()));
        } while (true);
      }

     protected:
      std::shared_ptr<model::Node> CreateNode(
          IteratorContext* ctx, model::Node::Args args) const override {
        return model::MakeSourceNode(std::move(args));
      }

      Status SaveInternal(IteratorStateWriter* writer) override {
        mutex_lock l(mu_);
        TF_RETURN_IF_ERROR(writer->WriteScalar(full_name("current_file_index"),
                                               current_file_index_));

        // `buffered_input_stream_` is empty if
        // 1. GetNext has not been called even once.
        // 2. All files have been read and iterator has been exhausted.
        if (buffered_input_stream_) {
          TF_RETURN_IF_ERROR(writer->WriteScalar(
              full_name("current_pos"), buffered_input_stream_->Tell()));
        }
        return Status::OK();
      }

      Status RestoreInternal(IteratorContext* ctx,
                             IteratorStateReader* reader) override {
        mutex_lock l(mu_);
        ResetStreamsLocked();
        int64 current_file_index;
        TF_RETURN_IF_ERROR(reader->ReadScalar(full_name("current_file_index"),
                                              &current_file_index));
        current_file_index_ = size_t(current_file_index);
        // The key "current_pos" is written only if the iterator was saved
        // with an open file.
        if (reader->Contains(full_name("current_pos"))) {
          int64 current_pos;
          TF_RETURN_IF_ERROR(
              reader->ReadScalar(full_name("current_pos"), &current_pos));

          TF_RETURN_IF_ERROR(SetupStreamsLocked(ctx->env()));
          TF_RETURN_IF_ERROR(buffered_input_stream_->Seek(current_pos));
        }
        return Status::OK();
      }

     private:
      // Sets up reader streams to read from the file at `current_file_index_`.
      Status SetupStreamsLocked(Env* env) EXCLUSIVE_LOCKS_REQUIRED(mu_) {
        if (current_file_index_ >= dataset()->filenames_.size()) {
          return errors::InvalidArgument(
              "current_file_index_:", current_file_index_,
              " >= filenames_.size():", dataset()->filenames_.size());
        }

        if (IsDataOwner()) {
          // Actually move on to next file.
          TF_RETURN_IF_ERROR(
            env->NewRandomAccessFile(dataset()->filenames_[current_file_index_], &file_));
          input_stream_ = absl::make_unique<io::RandomAccessInputStream>(file_.get(), false);

          if (dataset()->use_compression_) {
            zlib_input_stream_ = absl::make_unique<io::ZlibInputStream>(
              input_stream_.get(), dataset()->options_.input_buffer_size,
              dataset()->options_.input_buffer_size, dataset()->options_);
            buffered_input_stream_ = absl::make_unique<io::BufferedInputStream>(
              zlib_input_stream_.get(), dataset()->options_.input_buffer_size, false);
          } else {
            buffered_input_stream_ = absl::make_unique<io::BufferedInputStream>(
              input_stream_.get(), dataset()->options_.input_buffer_size, false);
          }

          log_debug << "data owner: " << net_io_->GetCurrentNodeId() << " and setup stream";
        } else {
          log_debug << "not data owner: " << net_io_->GetCurrentNodeId() << " with setup fake stream";
        }

        // [kelvin] Get data file info, eg. fields, lines, delmiter
        if (ExchangeDataFileInfo() != 0) {
          return errors::InvalidArgument(
              "current_file_index_:", current_file_index_,
              " Exchange data file description info failed");
        }

        is_setup_ = true;
        return Status::OK();
      }

      Status ReadNextLine(string& line_contents) {
        if (!is_setup_) {
          log_error << "not setup stream to iterator!";
          return Status(error::FAILED_PRECONDITION, "not setup stream");
        }

        Status s = Status::OK();
        line_contents.clear();
        if (IsDataOwner() && buffered_input_stream_) {
          bool is_blank = true;
          do {
            s = buffered_input_stream_->ReadLine(&line_contents);
            if (errors::IsOutOfRange(s) || current_file_line_count_ == data_file_info_.lines)
              return Status(error::OUT_OF_RANGE, "text file out of range");

            if (line_contents.find_first_not_of(' ') != std::string::npos)// not space line
            {
              current_file_line_count_++;
              break;
            }
          } while(is_blank);

          // log_debug << "owner read file line count:" << current_file_line_count_ << ", contents sizes:" << line_contents.size();
        } 
        else {
          // make a fake line
          if (current_file_line_count_ == data_file_info_.lines)
            return Status(error::OUT_OF_RANGE, "text file out of range");
          
          line_contents.resize(data_file_info_.fields*2-1);
          size_t i = 0;
          for ( ; i < data_file_info_.fields-1; ++i) {
            line_contents[2*i] = '0';
            line_contents[2*i+1] = data_file_info_.delim;
          }

          if (i == data_file_info_.fields-1)
          {  
            line_contents[data_file_info_.fields*2-2] = '0';
            ++current_file_line_count_;
          }

          // log_debug << "no-owner read file line count:" << current_file_line_count_ << ", contents sizes:" << line_contents.size();
        }

        return s;
      }

      int ExchangeDataFileInfo() {
        string node_id = net_io_->GetCurrentNodeId();
        log_debug << "to exchange data file info... node_id: " << node_id;

        // get unique msg key
        std::stringstream msg_key;
        msg_key << "/SecureTextDataset/" << current_file_index_ << "/" << dataset()->unique_op_name_;
        log_debug << "SecureTextDataset op msg key:" << msg_key.str() ;
        msg_id_t msg__msg_key(msg_key.str());

        // assemble data file info
        string result, msg;
        if (IsDataOwner()) {
          DataFileInfo file_info;
          string filename = dataset()->filenames_[current_file_index_];
          get_file_lines_fields(filename, ',', file_info.lines, file_info.fields);
          file_info.with_header = false;
          file_info.delim = ',';
          msg.append((char*)&file_info, sizeof(file_info));
          log_debug << "get_file_lines_fields: file lines=" << file_info.lines << ", file fields=" << file_info.fields ;
        } 
        else {
          msg.resize(sizeof(data_file_info_));
        }

        // dataowner send file info to peers and non-dataowner recv file info to peers
        if (0 != ProtocolManager::Instance()->GetProtocol(task_id_)->GetOps(msg__msg_key)->Broadcast(data_owner_, msg, result))
        {
          log_error << "call Broadcast failed, node id:  " << node_id ;
          return -1;
        }

        // save file info
        if (IsDataOwner())
          memcpy(&data_file_info_, msg.data(), sizeof(data_file_info_));
        else
          memcpy(&data_file_info_, result.data(), sizeof(data_file_info_));


        log_info << "node id:" << node_id << ", owner?:" << IsDataOwner() 
        << ", msgid:" << msg__msg_key << " " << " Succeed in reading data from CSV file." << " Total lines: "
        << data_file_info_.lines << ", Total fields: " << data_file_info_.fields << "."
        << ", msgid:" << msg__msg_key;
        return 0;
      }

      bool IsDataOwner() const {
        return data_owner_ == net_io_->GetCurrentNodeId();
      }

      // Resets all reader streams.
      void ResetStreamsLocked() EXCLUSIVE_LOCKS_REQUIRED(mu_) {
        if (IsDataOwner() && input_stream_) {
          input_stream_.reset();
          zlib_input_stream_.reset();
          buffered_input_stream_.reset();
          file_.reset();
        }
        is_setup_ = false;
      }

    private:
      mutex mu_;
      std::unique_ptr<io::RandomAccessInputStream> input_stream_
          GUARDED_BY(mu_);
      std::unique_ptr<io::ZlibInputStream> zlib_input_stream_ GUARDED_BY(mu_);
      std::unique_ptr<io::BufferedInputStream> buffered_input_stream_
          GUARDED_BY(mu_);
      size_t current_file_index_ GUARDED_BY(mu_) = 0;
      size_t current_file_line_count_ GUARDED_BY(mu_) = 0;
      DataFileInfo data_file_info_;
      std::unique_ptr<RandomAccessFile> file_
          GUARDED_BY(mu_);  // must outlive input_stream_
      string data_owner_;
      bool is_setup_ GUARDED_BY(mu_) = false;
      string task_id_ = "";
      shared_ptr<NET_IO> net_io_ = nullptr;
    };//Dataset::Iterator

    const std::vector<string> filenames_;
    const string compression_type_;
    const bool use_compression_;
    const io::ZlibCompressionOptions options_;
    string data_owner_ = "";
    string task_id_ = "";
    string unique_op_name_;
  };//Dataset

  string unique_op_name_;
};//PrivateTextLineDatasetOp

REGISTER_KERNEL_BUILDER(Name("PrivateTextLineDataset").Device(DEVICE_CPU),
                        PrivateTextLineDatasetOp);

}
}
}

