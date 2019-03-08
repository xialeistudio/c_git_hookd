FROM ubuntu
RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/' /etc/apt/sources.list
RUN sed -i 's/security.ubuntu.com/mirrors.aliyun.com/' /etc/apt/sources.list
RUN apt-get update
RUN apt-get install libevent-2.1-6 git -y -q
ADD git-hookd /usr/local/bin/git-hookd
ENTRYPOINT ["/usr/local/bin/git-hookd"]