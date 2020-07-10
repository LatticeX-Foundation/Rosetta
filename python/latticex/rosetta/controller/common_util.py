# ==============================================================================
# Copyright 2020 The LatticeX Foundation
# This file is part of the Rosetta library.
#
# The Rosetta library is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The Rosetta library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
# =============================================================================="
""" some auxiliary functionality for internal usage """

import os
import sys as _sys
import traceback as _traceback

import logging as _logging

import threading

# Note[George]: this is just a simplified copy from tensorflow's logging module
_logger = None
_logger_lock = threading.Lock()

def _get_caller(offset=3):
  """Returns a code and frame object for the lowest non-logging stack frame."""
  # Use sys._getframe().  This avoids creating a traceback object.
  # pylint: disable=protected-access
  f = _sys._getframe(offset)
  # pylint: enable=protected-access
  our_file = f.f_code.co_filename
  f = f.f_back
  while f:
    code = f.f_code
    if code.co_filename != our_file:
      return code, f
    f = f.f_back
  return None, None

# The definition of `findCaller` changed in Python 3.2
if _sys.version_info.major >= 3 and _sys.version_info.minor >= 2:
  def _logger_find_caller(stack_info=False):  # pylint: disable=g-wrong-blank-lines
    code, frame = _get_caller(4)
    sinfo = None
    if stack_info:
      sinfo = '\n'.join(_traceback.format_stack())
    if code:
      return (code.co_filename, frame.f_lineno, code.co_name, sinfo)
    else:
      return '(unknown file)', 0, '(unknown function)', sinfo
else:
  def _logger_find_caller():  # pylint: disable=g-wrong-blank-lines
    code, frame = _get_caller(4)
    if code:
      return (code.co_filename, frame.f_lineno, code.co_name)
    else:
      return '(unknown file)', 0, '(unknown function)'

def rtt_get_logger():
  """Return Rosetta logger instance."""
  global _logger

  # Use double-checked locking to avoid taking lock unnecessarily.
  if _logger:
    return _logger

  _logger_lock.acquire()
  try:
    if _logger:
      return _logger
    # Scope the Rosetta logger to not conflict with users' loggers.
    logger = _logging.getLogger('Rosetta')

    # Override findCaller on the logger to skip internal helper functions
    logger.findCaller = _logger_find_caller

    # Don't further configure the Rosetta logger if the root logger is
    # already configured. This prevents double logging in those cases.
    if not _logging.getLogger().handlers:
      _logging_target = _sys.stderr
      # Add the output handler.
      _handler = _logging.StreamHandler(_logging_target)
      _handler.setFormatter(_logging.Formatter(_logging.BASIC_FORMAT, None))
      logger.addHandler(_handler)

    _logger = logger
    return _logger
  finally:
    _logger_lock.release()

