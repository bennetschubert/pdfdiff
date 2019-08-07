# PDFDiff

Visually compare pdf pages

## Usage
`pdfdiff $pdf1 $pdf1Page $pdf2 $pdf2Page $outputfile`

- **$pdf1**: path to pdf file
- **$pdf1Page**: pageindex in pdf1
- **$pdf2**: path to pdf file
- **$pdf2Page**: pageindex in pdf2
- **$outputfile**: \*.png or \*.ppm file

**Attention** pageindex is starting at 0

**Example**
```bash
pdfdiff pdf1.pdf 0 pdf2.pdf 1 diff.png
```

Compares the first page of pdf1.pdf to the second page of pdf2.pdf.\
The result is stored in PNG-Format in the file diff.png.

## Build
### Inside Docker
```
# On host machine first build the docker image, then run a container
$~: ./build.sh
$~:	./run.sh

# now inside docker container
$/src: make
# run pdfdiff using ./pdfdiff $...args
```

## On your own Host 
### Prerequesites
- make
- gcc
- mupdf-libraries (package name might be mupdf-dev)

Once you got all the necessary tools and libs just run `make` inside the src directory.
