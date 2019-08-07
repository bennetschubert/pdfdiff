FROM alpine:latest

RUN apk add vim
RUN apk add gcc make
RUN apk add libc-dev musl-dev
RUN apk add mupdf-dev

CMD /bin/sh
