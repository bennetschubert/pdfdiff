FROM alpine:latest

RUN apk add gcc make libc-dev musl-dev mupdf-dev

CMD /bin/sh
