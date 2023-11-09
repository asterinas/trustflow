#!/bin/bash

make clean
env PYTHONPATH=$PYTHONPATH:$PWD/.. make SPHINXOPTS="-D language=zh_CN" html
