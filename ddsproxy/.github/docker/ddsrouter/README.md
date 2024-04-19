# DDS PROXY TEST DOCKER

In order to build this docker image, use command in current directory:

```sh
docker build --rm -t ddsproxy_test:some_tag --build-arg "fastdds_branch=master" --build-arg "devutils_branch=main" --build-arg "ddspipe_branch=main" --build-arg "ddsproxy_branch=main" .
```
