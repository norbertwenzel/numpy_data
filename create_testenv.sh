#!/bin/sh

docker pull ubuntu:16.04
docker pull ubuntu:14.04
docker build -f docker/Dockerfile.xenial -t numpy_data/xenial .
docker build -f docker/Dockerfile.trusty -t numpy_data/trusty .
