/*
	Copyright © 2020 daltomi <daltomi@disroot.org>

	This file is part of xsv.

	xsv is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xsv is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xsv.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <FL/Fl_Image.H>
#include <FL/Fl.H>
#include <FL/Fl_PNG_Image.H>

static FILE* file = stdout;
static char name_data[40 + 6]; // +icon_
static char input[40];
static Fl_Image* img = nullptr;


// Basado en FLUID (FLTK)
static void write_img_data()
{
	size_t const size = (img->w() * img->d() + img->ld()) * img->h();
	unsigned char const* w = (unsigned char const*)img->data()[0];
	unsigned char const* e = w + size;
	int line = 1;

	for (; w < e; )
	{
		unsigned char c = *w++;
		if (c > 99)
		{
			line += 4;
		}
		else if (c > 9)
		{
			line += 3;
		}
		else
		{
			line += 2;
		}

		if (line++ >= 77)
		{
			fputs("\n", file); line = 0;
		}

		fprintf(file, "%d", c);

		if( w < e)
		{
			putc (',',file);
		}
	}
	fputs("};\n", file);
}


static void write_img_property()
{
	fprintf (file,"static Fl_Image * get_icon_%s () {\n"
			"\tstatic Fl_Image * img = new Fl_RGB_Image"
			"(icon_%s, %d, %d, %d, %d);\n"
			"\treturn img;\n}\n",
			input, input, img->w(), img->h(), img->d(), img->ld());
}


Fl_Image* open_image()
{
	//TODO: soportar otros formatos.
	Fl_PNG_Image* img = new Fl_PNG_Image(input);

	if (img->fail())
	{
		fprintf (stderr, "Error. no se pudo abrir la imagen %s.\n", input);
		delete img;
		exit (EXIT_FAILURE);
	}

	return img;
}


void write_name_data()
{
	snprintf(name_data, sizeof(name_data), "icon_%s", input);
	fprintf(file, "static unsigned char %s [] = {\n", name_data);
}


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Error. faltan parametros.\n");
		return EXIT_FAILURE;
	}

	strncpy(input, argv[1], 40);

	img = open_image();

	// Quitar la extension de archivo.
	*(char*)(strchr(input, '.')) = '\0';

	write_name_data();
	write_img_data();
	write_img_property();
	delete img;

	return EXIT_SUCCESS;
}
