#!/bin/bash
set -e

VERSION=$(cat docker/VERSION)
make release

IMAGE_NAME=sunzhenkai/kfpanda
docker build -t ${IMAGE_NAME}:${VERSION} -f docker/Dockerfile .

echo "push ${IMAGE_NAME}:${VERSION} to docker hub? [y/N]"
read choice

case $choice in
y)
  docker push ${IMAGE_NAME}:${VERSION}
  ;;
n)
  echo "skip publishing image"
  ;;
*)
  echo "enter y/n"
  ;;
esac
