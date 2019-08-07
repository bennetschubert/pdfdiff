docker run --rm --mount type=bind,source="$(pwd)"/src/,target=/src/ --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -w /src -it pdfdiff
