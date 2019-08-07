FROM alpine:latest

RUN apk add gcc make libc-dev musl-dev mupdf-dev freetype-dev jbig2dec-dev harfbuzz-dev openjpeg-dev jpeg-dev

CMD /bin/sh
