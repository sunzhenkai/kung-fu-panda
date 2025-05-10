FROM ubuntu:24.04

WORKDIR /app/kfpanda
SHELL ["/bin/bash", "--login", "-c"]
COPY release/kfpanda bin/kfpanda
COPY docker/entrypoint.sh .

EXPOSE 9820
ENTRYPOINT ["bash", "entrypoint.sh"]

