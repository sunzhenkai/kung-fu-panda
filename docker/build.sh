#!/bin/bash
set -e

VERSION=$(cat docker/VERSION)
make release

IMAGE_NAME=sunzhenkai/kfpanda
docker build -t ${IMAGE_NAME}:${VERSION} -f docker/Dockerfile .
#docker push ${IMAGE_NAME}:${VERSION}
