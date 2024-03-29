#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mupdf/fitz.h"

enum OUTPUT_TYPE {OUT_PAM, OUT_PNG};

char* get_file_extension(char* filename){
	char* dot = strrchr(filename, '.');
	if(!dot || dot == filename) return "";
	return dot + 1;
}

fz_pixmap* pixmap_compare(fz_context* ctx, fz_pixmap* dest, fz_pixmap* src){
	int src_width, src_height, dest_width, dest_height;
	fz_colorspace* colorspace;
	fz_pixmap* pix;

	fz_try(ctx) {
		src_width = fz_pixmap_width(ctx, src);
		src_height = fz_pixmap_height(ctx, src);
		dest_width = fz_pixmap_width(ctx, dest);
		dest_height = fz_pixmap_height(ctx, dest);
		if(!(src_width == dest_width && src_height == dest_height))
			fz_throw(
				ctx,
				FZ_ERROR_GENERIC,
				"error: input widths and height must be equal"
			);

		//copy the colorspace from src
		colorspace = fz_pixmap_colorspace(ctx, src);
		//create a new empty pixmap for the result
		pix = fz_new_pixmap(ctx, colorspace, src_width, src_height, NULL, 1);
	}
	fz_catch(ctx) return NULL;

	//finally do the comparison
	unsigned char *a, *b, *out;
	for (int i = 0; i < src_height * src_width; ++i)
	{
		a = &src->samples[i * src->n];
		b = &dest->samples[i * dest->n];
		out = &pix->samples[i * pix->n];

		if(memcmp(a, b, 3) != 0){
			out[0] = 0xFF; //r
			out[1] = 0x00; //g
			out[2] = 0x00; //b
			out[3] = 0x99; //a
		}
	}

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

	int output_type = -1;

	fz_context *ctx;
	fz_document *doc1, *doc2;
	fz_pixmap *pix1, *pix2, *cmp_pix;

	fz_output *output = NULL;
	void (*output_fn)(fz_context *ctx, fz_output* output, fz_pixmap* pix);//function to used for pixmap output

	if(!sscanf(page_arg_1, "%d", &input_page_1)){
		fprintf(stderr, "invalid argument page, exepected (int), got: %s", page_arg_1);
		return EXIT_FAILURE;
	}

	if(!sscanf(page_arg_2, "%d", &input_page_2)){
		fprintf(stderr, "invalid argument page, exepected (int), got: %s", page_arg_2);
		return EXIT_FAILURE;
	}

	//create fitz context
	ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if(!ctx){
		fprintf(stderr, "cannot create mupdf context");
		fz_drop_context(ctx);
		return EXIT_FAILURE;
	}

	//check output file format
	if(strcmp(output_file,"-png") == 0) {
		output_type = OUT_PNG;
		output = fz_stdout(ctx);
	}
	else if(strcmp(output_file,"-pam") == 0) {
		output_type = OUT_PAM;
		output = fz_stdout(ctx);
	}
	else {
		char* extension = get_file_extension(output_file);
		if (strcmp(extension,"png") == 0) output_type = OUT_PNG;
		if (strcmp(extension,"pam") == 0) output_type = OUT_PAM;
		output = fz_new_output_with_path(ctx, output_file, 0);
	}

	switch(output_type){
		case OUT_PNG:
			output_fn = (void (*)(fz_context*, fz_output*, fz_pixmap*)) &fz_write_pixmap_as_png;
			break;
		case OUT_PAM:
			output_fn = (void (*)(fz_context*, fz_output*, fz_pixmap*)) &fz_write_pixmap_as_pam;
			break;
		default: 
			fz_drop_context(ctx);
			fprintf(stderr, "could not infer type of output file: %s\n", output_file);
			return EXIT_FAILURE;
	}

	fz_try(ctx) {
		fz_register_document_handlers(ctx);

		//open documents
		doc1 = fz_open_document(ctx, input_file_1);
		doc2 = fz_open_document(ctx, input_file_2);

		//render pages
		pix1 = fz_new_pixmap_from_page_number(ctx, doc1, input_page_1, fz_identity, fz_device_rgb(ctx), 0);
		pix2 = fz_new_pixmap_from_page_number(ctx, doc2, input_page_2, fz_identity, fz_device_rgb(ctx), 0);

		//compare the images
		cmp_pix = pixmap_compare(ctx, pix1, pix2);
		if(!cmp_pix) fz_throw(ctx, FZ_ERROR_GENERIC, "could not compare images\n");

		//output the result
		(*output_fn)(ctx, output, cmp_pix);
	}
	fz_always(ctx){
		//clean stuff up
		fz_drop_pixmap(ctx, pix1);
		fz_drop_pixmap(ctx, pix2);
		fz_drop_pixmap(ctx, cmp_pix);
		fz_drop_document(ctx, doc1);
		fz_drop_document(ctx, doc2);
		fz_drop_context(ctx);
	}
	fz_catch(ctx){
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
