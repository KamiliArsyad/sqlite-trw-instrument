#!/bin/bash

TARGET_DIR=$1
PROJ_DIR=~/sqlite-rw-instrument

cp ${PROJ_DIR}/cmake-build-debug/sqlite_rw_instrument .
cp -r ${PROJ_DIR}/tests/* .

if ! ${TARGET_DIR}; then
  echo "test dir specified at ${TARGET_DIR}"
  cp -r ./* "${TARGET_DIR}"
fi