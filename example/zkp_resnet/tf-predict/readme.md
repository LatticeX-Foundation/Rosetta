## Usage Example for plaintext inference on testing dataset on the plaintext model

### test with ResNet-50 model
#### Note: please first copy your model trained from `tf-train` scripts to checkpoint/ResNet50_cifar10 sub-directory.  
python3 main.py --phase test --res_n 50 --dataset cifar10 --test_size 10000

### test with ResNet-101 model
#### Note: please first copy your model trained from `tf-train` scripts to checkpoint/ResNet101_cifar10 sub-directory.  
python3 main.py --phase test --res_n 101 --dataset cifar10 --test_size 10000