#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mupdf/fitz.h"

enum OUTPUT_TYPE {OUT_PPM, OUT_PNG};

char* get_file_extension(char* filename){
	char* dot = strrchr(filename, '.');
	if(!dot || dot == filename) return "";
	return dot + 1;
}

int print_pdf_info(fz_context* ctx, fz_document* doc){
	int page_count; 

	//count number of pages
	fz_try(ctx) page_count = fz_count_pages(ctx, doc);
	fz_catch(ctx) {
		return EXIT_FAILURE;
	}

	printf("Pages: %d\n", page_count);
	return EXIT_SUCCESS;
}

int pixmap_to_ppm(char* filename, fz_pixmap* pix){
	FILE* fd;
	fd = fopen(filename, "w");
	if(!fd){
		fclose(fd);
		return EXIT_FAILURE;
	}

	int err = 0;
	err |= fprintf(fd, "P3\n");
	err |= fprintf(fd, "%d %d\n", pix->w, pix->h);
	err |= fprintf(fd, "255\n");
	for (int y = 0; y < pix->h ; ++y)
	{
		unsigned char *p = &pix->samples[y * pix->stride];
		for (int x = 0; x < pix->w; ++x)
		{
			if (x > 0) err |= 	fprintf(fd, "  ");
			err |=  fprintf(fd, "%3d %3d %3d", p[0], p[1], p[2]);
			p += pix->n;
		}
		err |= fprintf(fd, "\n");
	}

	fclose(fd);

	if(err < 0) return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

int pixmap_to_png(char * filename, fz_context* ctx, fz_pixmap* pix){
	fz_try(ctx) fz_save_pixmap_as_png(ctx, pix ,filename);
	fz_catch(ctx){
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/*
 *1. create empty pixmap
 *2. draw black pixel for each difference in the images
 *3. return the pixmap
 */
fz_pixmap* pixmap_compare(fz_pixmap* dest, fz_pixmap* src, fz_context* ctx){
	//fprintf(stderr, "Functin pixmap_compare not implemented yet\n");
	
	int src_width, src_height, dest_width, dest_height;
	fz_pixmap* pix;

	fz_try(ctx) {
		src_width = fz_pixmap_width(ctx, src);
		src_height = fz_pixmap_height(ctx, src);
		dest_width = fz_pixmap_width(ctx, dest);
		dest_height = fz_pixmap_height(ctx, dest);
	}
	fz_catch(ctx) return NULL;

	if(!(src_width == dest_width && src_height == dest_height)){
		fprintf(stderr, "input mismatch got: %dx%dpx and %dx%dpx\n",
				src_width, src_height, dest_width, dest_height);
		return NULL;
	}

	fz_colorspace* colorspace;
	fz_try(ctx) colorspace = fz_pixmap_colorspace(ctx, src);
	fz_catch(ctx) return NULL;

	fz_try(ctx) pix = fz_new_pixmap(ctx, colorspace, src_width, src_height, NULL, 1);
	fz_catch(ctx) return NULL;

	//finally do the comparison
	//int total_pixels = src_width * src_height;
	//int equal_pixels = 0;

	for (int y = 0; y < src_height; ++y)
	{
		for (int x = 0; x < src_width; ++x)
		{
			int offset = (y * src_width) + x;
			unsigned char *a = &src->samples[offset * src->n];
			unsigned char *b = &dest->samples[offset * dest->n];
			unsigned char *out = &pix->samples[offset * pix->n];
			if(memcmp(a, b, 3) == 0){} //equal_pixels++;
			else {
				out[0] = 0xFF; //r
				out[1] = 0x00; //g
				out[2] = 0x00; //b
				out[3] = 0x99; //a
			}
		}
	}

	//fprintf(stderr, "%d of %d pixels are equal(%.2f%%)\n", equal_pixels, total_pixels, (float)equal_pixels / (float)total_pixels * 100.0f);
	return pix;
}

int main(int argc, char* argv[]){
	if(argc != 6){
		fprintf(stderr, "expected 5 parameters $file $page $file $page $outputfile, got: %d\n", argc-1);
		return EXIT_FAILURE;
	}

	char* input_file_1 = argv[1];
	char* page_arg_1 = argv[2];
	char* input_file_2 = argv[3];
	char* page_arg_2 = argv[4];
	char* output_file = argv[5];

	int input_page_1, input_page_2;

	char* extension;
	int output_type;

	fz_context *ctx;
	fz_document *doc1, *doc2;
	fz_pixmap *pix1, *pix2, *cmp_pix;

	if(!sscanf(page_arg_1, "%d", &input_page_1)){
		fprintf(stderr, "invalid argument page, exepected (int), got: %s", page_arg_1);
		return EXIT_FAILURE;
	}

	if(!sscanf(page_arg_2, "%d", &input_page_2)){
		fprintf(stderr, "invalid argument page, exepected (int), got: %s", page_arg_2);
		return EXIT_FAILURE;
	}

	extension = get_file_extension(output_file);
	//checkout output file format
	if (strcmp(extension, "png") == 0 )
		output_type = OUT_PNG;
	else if (strcmp(extension, "ppm") == 0 )
		output_type = OUT_PPM;
	else {
		fprintf(stderr, "could not infer type of output file: %s\n", output_file);
		return EXIT_FAILURE;
	}

	//create fitz context
	ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if(!ctx){
		fprintf(stderr, "cannot create mupdf context");
		fz_drop_context(ctx);
		return EXIT_FAILURE;
	}

	fz_try(ctx) fz_register_document_handlers(ctx);
	fz_catch(ctx) {
		fz_drop_context(ctx);
		return EXIT_FAILURE;
	}

	//open document
	fz_try(ctx) doc1 = fz_open_document(ctx, input_file_1);
	fz_catch(ctx) {
		fz_drop_context(ctx);
		return EXIT_FAILURE;
	}

	fz_try(ctx) doc2 = fz_open_document(ctx, input_file_2);
	fz_catch(ctx) {
		fz_drop_document(ctx, doc1);
		fz_drop_context(ctx);
		return EXIT_FAILURE;
	}

	//render page
	fz_try(ctx) pix1 = fz_new_pixmap_from_page_number(ctx, doc1, input_page_1, &fz_identity, fz_device_rgb(ctx), 0);
	fz_catch(ctx) {
		fz_drop_document(ctx, doc1);
		fz_drop_document(ctx, doc2);
		fz_drop_context(ctx);
		return EXIT_FAILURE;
	}

	fz_try(ctx) pix2 = fz_new_pixmap_from_page_number(ctx, doc2, input_page_2, &fz_identity, fz_device_rgb(ctx), 0);
	fz_catch(ctx) {
		fz_drop_document(ctx, doc1);
		fz_drop_document(ctx, doc2);
		fz_drop_context(ctx);
		return EXIT_FAILURE;
	}

	cmp_pix = pixmap_compare(pix1, pix2, ctx);
	if(!cmp_pix) fprintf(stderr, "could not compare images\n");

	int write_error = -1;
	switch(output_type){
	case OUT_PNG:
		write_error = pixmap_to_png(output_file, ctx, cmp_pix);
		break;
	case OUT_PPM:
		write_error = pixmap_to_ppm(output_file, cmp_pix);
		break;
	default: ;
	}

	if(write_error){
		fz_drop_pixmap(ctx, pix1);
		fz_drop_pixmap(ctx, pix2);
		fz_drop_document(ctx, doc1);
		fz_drop_document(ctx, doc2);
		fz_drop_context(ctx);
		fprintf(stderr, "error while writing, output may be corrupted\n");
		return EXIT_FAILURE;
	}
	
	fz_drop_pixmap(ctx, pix1);
	fz_drop_pixmap(ctx, pix2);
	fz_drop_pixmap(ctx, cmp_pix);
	fz_drop_document(ctx, doc1);
	fz_drop_document(ctx, doc2);
	fz_drop_context(ctx);
	return EXIT_SUCCESS;
}
