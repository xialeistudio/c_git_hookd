FROM busybox
ADD git-hookd /usr/local/bin/git-hookd
ENTRYPOINT ["/usr/local/bin/git-hookd"]