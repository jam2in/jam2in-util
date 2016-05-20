### Scripts for setting up Zookeeper and Arcus Cloud

replication 버전에 맞는 setup script 들을 생성해 주는 쉘 스크립트입니다.
zk_repl.sh 파일의 내용만 원하는 대로 편집하여 사용하시면 됩니다.

arcus 서버가 설치된 디렉토리를 지정해 주면, 그 아래에 scripts 디렉토리를 생성하고
3종류의 스크립트를 생성합니다.
- setup-* : zookeeper 에 node 를 등록하는 script.
- remove-* : zookeeper 에 등록된 node 를 제거하는 script.
- start-* : memcached process 를 시작하는 script.
