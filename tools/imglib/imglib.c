
#include "imglib.h"

/*
 * The buffer_xxx functions operate on arbitrary buffers.  You can put
 * anything you want in there.  It's simply a convenient way of tracking
 * data plus a length.  There are a few other things in there for
 * convenience.
 *
 * The bitmap_xxx functions work on buffers, too, but assume that each
 * byte corresponds to a single pixel.  Thus, you can have up to 256
 * colors in a bitmap.  Bitmaps are laid out from row 1 to row N,
 * with the columns in each row given left to right.  Bitmaps require
 * that the 'width' and 'height' fields be set in the buffer so that
 * correlation between (x,y) locations are byte offsets is possible.
 *
 * A frame is just a bitmap of size 128x32, the size of the entire
 * WPC dot matrix display.
 *
 * A joined bitmap is one in which each group of 8 adjacent pixels within
 * one row are combined into a single byte.  Joining requires a bitmap
 * of a single color only (each pixel is either 00h or 01h).  This is the
 * format that can actually be copied directly to the DMD memory.
 */

struct buffer *buffer_alloc (unsigned int maxlen)
{
	struct buffer *buf = malloc (sizeof (struct buffer));
	buf->len = maxlen;
	buf->data = buf->_data;
	buf->hist = NULL;
	buf->width = buf->height = 0;
	buf->color = 1;
	memset (buf->_data, 0, maxlen);
	return buf;
}


struct buffer *buffer_copy (struct buffer *buf)
{
	struct buffer *copy = buffer_alloc (buf->len);
	copy->width = buf->width;
	copy->height = buf->height;
	copy->color = buf->color;
	memcpy (copy->_data, buf->_data, buf->len);
	if (buf->hist)
		histogram_update (copy);
	return copy;
}


struct buffer *bitmap_alloc (unsigned int width, unsigned int height)
{
	struct buffer *buf = buffer_alloc (width * height);
	buf->width = width;
	buf->height = height;
	return buf;
}


struct buffer *frame_alloc (void)
{
	return bitmap_alloc (FRAME_WIDTH, FRAME_HEIGHT);
}

struct layer *layer_alloc (struct buffer *bitmap)
{
	struct layer *layer = malloc (sizeof (struct layer));
	layer->bitmap = bitmap;
	layer->buf = frame_alloc ();
	layer->coord.x = layer->coord.y = 0;
	return layer;
}

unsigned int bitmap_pos (struct buffer *buf, unsigned int x, unsigned int y)
{
	return y * buf->width + x;
}


void buffer_read (struct buffer *buf, FILE *fp)
{
	buf->len = fread (buf->data, sizeof (U8), MAX_BUFFER_SIZE, fp);
}

void buffer_write (struct buffer *buf, FILE *fp)
{
	fwrite (buf->data, sizeof (U8), buf->len, fp);
}

void bitmap_write_ascii (struct buffer *buf, FILE *fp)
{
	unsigned int x, y;

	if (buf->height * buf->width < buf->len)
	{
		fprintf (stderr, "error: cannot render %dx%d ASCII image with only %d bytes\n",
			buf->width, buf->height, buf->len);
		exit (1);
	}

	for (y = 0; y < buf->height; y++)
	{
		for (x = 0; x < buf->width; x++)
			if (buf->data[bitmap_pos (buf, x, y)] != 0)
				fputc ('X', fp);
			else
				fputc ('.', fp);
		fputc ('\n', fp);
	}
}

void buffer_write_c (struct buffer *buf, FILE *fp)
{
	unsigned int off;

	if (buf->hist)
	{
		fprintf (fp, "/* Histogram (%d unique values)\n", buf->hist->unique);
		if (buf->hist->unique < 16)
		{
			for (off = 0; off < buf->hist->unique; off++)
			{
				U8 val = buf->hist->most_frequent[off];
				unsigned int count = buf->hist->count[val];
				fprintf (fp, " * %d occurrences of %02X\n",
					count, val);
			}
		}
		fprintf (fp, " */\n");
	}

	fprintf (fp, "/* %dx%d %d bytes */\n", buf->width, buf->height, buf->len);

	for (off = 0; off < buf->len; off++)
	{
		if (!(off % 16))
			fprintf (fp, "\n   ");
		fprintf (fp, "0x%02X,", buf->data[off]);
	}
	fprintf (fp, "\n");
}


/* Writes a 'joined bitmap' to an XBM. */
void buffer_write_xbm (struct buffer *buf, const char *ident, FILE *fp)
{
	fprintf (fp, "#define %s_width %d\n", ident, buf->width);
	fprintf (fp, "#define %s_height %d\n", ident, buf->height);
	fprintf (fp, "const unsigned char %s[] = {\n", ident);
	buffer_write_c (buf, fp);
	fprintf (fp, "}\n");
}


void buffer_write_pgm (struct buffer *buf, const char *ident, FILE *fp)
{
	unsigned int row, col;
	fprintf (fp, "P2\n");
	fprintf (fp, "#\n");
	fprintf (fp, "%d %d\n", buf->width, buf->height);
	for (row=0; row < buf->height; row++)
		for (col=0; col < buf->width; col++)
			fprintf (fp, "%d\n", buf->data[bitmap_pos (buf, col, row)]);
}


void cdecl_begin (const char ident[], FILE *fp)
{
	fprintf (fp, "unsigned char %s[] = {", ident);
}

void cdecl_end (FILE *fp)
{
	fprintf (fp, "};\n\n");
}


void buffer_free (struct buffer *buf)
{
	if (buf->hist)
		free (buf->hist);
	free (buf);
}

void layer_free (struct layer *layer)
{
	/* note: layer->bitmap is not freed */
	free (layer->buf);
	free (layer);
}


U8 xor_operator (U8 a, U8 b)
{
	return a ^ b;
}

U8 and_operator (U8 a, U8 b)
{
	return a & b;
}

U8 com_operator (U8 a)
{
	return ~a;
}

/** Performs a binary operation on two buffers.
 * Byte-by-byte, the operation is applied to each pair of corresponding
 * bytes. */
struct buffer *buffer_binop (struct buffer *a, struct buffer *b,
	binary_operator op)
{
	unsigned int off = 0;
	struct buffer *res = buffer_alloc (max (a->len, b->len));

	while (off < a->len && off < b->len)
	{
		res->data[off] = op (a->data[off], b->data[off]);
		off++;
	}
	return res;
}


struct buffer *buffer_unop (struct buffer *buf, unary_operator op)
{
	unsigned int off = 0;
	struct buffer *res = buffer_alloc (buf->len);

	while (off < buf->len)
	{
		res->data[off] = op (buf->data[off]);
		off++;
	}
	return res;
}


struct buffer *buffer_compute_delta (struct buffer *dst, struct buffer *src)
{
	return buffer_binop (dst, src, xor_operator);
}


struct buffer *buffer_joinbits (struct buffer *buf)
{
	unsigned int off, bit;
	struct buffer *res = buffer_alloc (buf->len / 8);

	for (off = 0; off < buf->len; off++)
	{
		U8 val = 0;
		for (bit = 0; bit < 8; bit++)
			if (buf->data[off * 8 + bit] == 1)
				val |= 1 << bit;
		res->data[off] = val;
	}
	return res;
}

struct buffer *buffer_splitbits (struct buffer *buf)
{
	unsigned int off, bit;
	struct buffer *res = buffer_alloc (buf->len * 8);

	for (off = 0; off < buf->len; off++)
	{
		for (bit = 0; bit < 8; bit++)
		{
			if (buf->data[off] & (1 << bit))
				res->data[off * 8 + bit] = 1;
			else
				res->data[off * 8 + bit] = 0;
		}
	}
	return res;
}


int buffer_compare (struct buffer *a, struct buffer *b)
{
	unsigned int off;

	if (a->len != b->len)
		return 1;
	for (off = 0; off < a->len; off++)
		if (a->data[off] != b->data[off])
		{
#ifdef DEBUG
			printf ("difference at offset %d (%02X vs. %02X)\n",
				off, a->data[off], b->data[off]);
#endif
			return 1;
		}
	return 0;
}


struct buffer *buffer_replace (struct buffer *old, struct buffer *new)
{
	free (old);
	return new;
}


struct histogram *histogram_update (struct buffer *buf)
{
	struct histogram *hist;
	unsigned int off;
	unsigned int i;
	int largest;
	unsigned int already_taken[256] = { 0, };

	if (!buf->hist)
		hist = buf->hist = malloc (sizeof (struct histogram));

	hist->unique = 0;
	for (off = 0; off < 256; off++)
		hist->count[off] = 0;

	for (off = 0; off < buf->len; off++)
	{
		if (hist->count[buf->data[off]] == 0)
			hist->unique++;
		hist->count[buf->data[off]]++;
	}

	for (i = 0; i < hist->unique; i++)
	{
		/* Find the ith largest value in the list that hasn't already
		been selected */
		largest = -1;

		/* Iterate through the count data, looking for the highest
		entry that has not already been selected from previous loops. */
		for (off = 0; off < 256; off++)
		{
			/* Always consider the first entry, but then only consider
			entries larger than previous ones. */
			if ((largest == -1) || (hist->count[off] > hist->count[largest]))
			{
				/* But skip entries that were taken before */
				if (!already_taken[off])
					largest = off;
			}
		}
		hist->most_frequent[i] = largest;
		already_taken[largest] = 1;
	}

	return hist;
}


/********************************************************************/

/* Compression scheme:

   Byte 0 of the compressed buffer contains flags:
		7:6	Init method
			Says how to initialize the page buffer before
			decompressing:
				00 = No initialization.
				01 = Copy previous image.
				10 = Clean page with all zeroes.
				11 = Reserved.
		5:4	Encryption method
			Says how to decrypt the following bytes:
				00 = No decryption.  Bytes are copied as-is.
						512 bytes must follow.
            01 = Run-Length Encoding.  512 bytes are
						described, but runs of consecutive bytes
						are compressed into a value/count pair.
				10 = Palette Encoding.  This is used when the
				      number of unique bytes in the image is small.
						The palette format is shown below.
				11 = Reserved.
		3		Patch flag
			When this bit is set, after decryption one or more
			'patches' are to be applied.  These are offset/value
			pairs which are used to fixup certain locations.
			This is intended for cases where RLE could be improved
			by damaging the input data in certain ways; and these
			patches are corrections to fix the damage.

	Beginning at byte 1, the encryption method as specified
	in [5:4] is run.  This area is variable-sized.  There is no
	explicit length given here.

	Following the encoded area, if bit 3 is set, are one or more
	patches.  Each patch is 3-bytes in length:
		Byte 0 = offset from previous patch, in bytes.  Up to
			255 positions can be advanced this way.  If more is
			needed, a 'fake patch' that patches identical data as
			before is required.  An offset of zero means no
			more patches, and the following bytes do not occur.
		Byte 1/2 = the 16-bit value to be placed in the image.

	When the init method is 'copy from previous', this affects
	how data and patches are applied.  In this case, we use XOR
	operations rather than plain stores.

   When using the palette method, the data layout is as follows:
	   Byte 1:	Number of palette entries (at most 16)
		Byte 2:  Palette - depending on the length above, each
		         byte here maps to a 2-bit (if 4 or less) or 4-bit
					encoded value.
	   Byte 2+N: The encoded data, either 2 or 4 bytes/encoded byte.
 */

#define CH_INIT_NONE 0x00
#define CH_INIT_COPY 0x40
#define CH_INIT_CLEAN 0x80
#define CH_INIT_MASK 0xC0

#define CH_ENCODE_NONE 0x00
#define CH_ENCODE_RLE 0x10
#define CH_ENCODE_PALETTE 0x20
#define CH_ENCODE_MASK 0x30

#define CH_PATCH 0x08

static U8 *buffer_write_run (U8 *ptr, U8 sentinel, U8 data, unsigned int count)
{
	if (count == 0);
	else if (count < 4)
	{
		do {
			*ptr++ = data;
		} while (--count > 0);
	}
	else
	{
		*ptr++ = sentinel;
		*ptr++ = data;
		*ptr++ = count;
	}

	return ptr;
}


/**
 * Return the estimated size of the image if it were compressed using
 * the palette method.
 */
static unsigned int palette_compression_length (struct histogram *hist)
{
	if (hist->unique > 16)
		return 10000;

	else if (hist->unique <= 4)
		return (FRAME_BYTE_SIZE / 4) + hist->unique + 1 + 1;

	else
		return (FRAME_BYTE_SIZE / 2) + hist->unique + 1 + 1;
}


/**
 * Given a joined bitmap, return a compressed version.
 * 'buf' is the buffer to be compressed.
 *
 * If 'prev' is non-NULL, it represents the previous image in the
 * animation.  The compressor will see if the new image can be
 * represented better as a delta from the previous.
 */
struct buffer *buffer_compress (struct buffer *buf, struct buffer *prev)
{
	struct buffer *rle;
	U8 *rleptr;
	unsigned int last, last_count;
	int n;
	U8 sentinel;

	/* Update the image histogram */
	histogram_update (buf);

	/* Find a byte value that does not occur in the image */
	for (n = 0; n <= 0xFF; n++)
	{
		if (buf->hist->count[n] == 0)
		{
			sentinel = n;
			break;
		}
	}

	/* Compute the run length encoded version of the buffer.
	We'll assume for now that just RLE is good enough.
	We will check that assumption later and see how good it really was. */
	rle = buffer_alloc (buf->len);
	rleptr = rle->data;
	last = 1000;
	last_count = 0;

	*rleptr++ = CH_ENCODE_RLE;
	*rleptr++ = sentinel;

	for (n = 0; n < buf->len ; n++)
	{
		if ((buf->data[n] == last) && (last_count < 255))
		{
			last_count++;
		}
		else
		{
			rleptr = buffer_write_run (rleptr, sentinel, last, last_count);
			last = buf->data[n];
			last_count = 1;
		}
	}
	rleptr = buffer_write_run (rleptr, sentinel, last, last_count);
	rle->len = rleptr - rle->data;

	/* See if delta encoding is better */
	if (prev != NULL)
	{
		struct buffer *delta = buffer_compress (prev, NULL);
		if (delta->len < rle->len)
		{
			/* TODO : it's possible that it needs to be _significantly_
			smaller for this to be worthwhile... */
		}
	}

#ifdef DEBUG
	printf ("Unique values in original = %d\n", buf->hist->unique);
	printf ("RLE encoded version = %d bytes\n", rle->len);
	printf ("Palette compression = %d bytes\n",
		palette_compression_length (buf->hist));
#endif
	return rle;
}


/**
 * Given a compressed bitmap, return the uncompressed, joined buffer.
 */
struct buffer *buffer_decompress (struct buffer *buf)
{
	struct buffer *res;
	U8 flags;
	U8 sentinel;
	U8 *inptr = buf->data;
	U8 *outptr;
	U8 val, count;

	res = buffer_alloc (MAX_BUFFER_SIZE);
	outptr = res->data;

	flags = *inptr++;

	switch (flags & CH_INIT_MASK)
	{
		case CH_INIT_NONE:
		case CH_INIT_COPY:
			break;

		case CH_INIT_CLEAN:
			break;
	}

	if ((flags & CH_ENCODE_MASK) == CH_ENCODE_RLE)
	{
		sentinel = *inptr++;
		while (outptr - res->data < FRAME_BYTE_SIZE)
		{
			val = *inptr++;
			if (val == sentinel)
			{
				val = *inptr++;
				count = *inptr++;
				do {
					*outptr++ = val;
				} while (--count > 0);
			}
			else
			{
				*outptr++ = val;
			}
		}
	}

	if (flags & CH_PATCH)
	{
	}

	res->len = outptr - res->data;
	return res;
}


/********************************************************************/


struct buffer *bitmap_crop (struct buffer *buf)
{
	struct buffer *res = buffer_alloc (buf->len);
	return res;
}

void bitmap_draw_pixel (struct buffer *buf, unsigned int x, unsigned int y)
{
	unsigned int pos = bitmap_pos (buf, x, y);
	if (pos < buf->len)
		buf->data[pos] = buf->color;
}


struct buffer *bitmap_paste (struct buffer *dst, struct buffer *src,
	unsigned int xoff, unsigned int yoff)
{
	unsigned int sx, sy;
	for (sx = 0; sx < src->width; sx++)
	{
		for (sy = 0; sy < src->height; sy++)
		{
			U8 pixel = src->data[bitmap_pos(src, sx, sy)];
			bitmap_draw_pixel (dst, sx + xoff, sy + yoff);
		}
	}
}


struct buffer *bitmap_tile (struct buffer *dst, struct buffer *src)
{
	unsigned int dx, dy;
	for (dx = 0; dx < dst->width; dx++)
	{
		for (dy = 0; dy < dst->height; dy++)
		{
			U8 pixel = src->data[bitmap_pos(src, dx % src->width, dy % src->height)];
			bitmap_draw_pixel (dst, dx, dy);
		}
	}
}


void bitmap_draw_line (struct buffer *buf,
	int x1, int y1,
	int x2, int y2)
{
	float xm, ym;

	/* The approach used here is recursive divide-and-conquer:
	1) Calculate the midpoint of the line to be drawn.
	2) Draw a line from one end to the midpoint.
	3) Draw a line from the other end to the midpoint.

	The recursion is terminated by looking for cases where
	the line is to be drawn is only 1 or 2 pixels in width. */

	if (abs (x2-x1) <= 1 && abs (y2-y1) <= 1)
	{
		bitmap_draw_pixel (buf, x1, y1);
		bitmap_draw_pixel (buf, x1, y2);
		bitmap_draw_pixel (buf, x2, y1);
		bitmap_draw_pixel (buf, x2, y2);
		return;
	}

	/* Find the midpoint of the line */
	xm = (x1 + x2 + 0.5) / 2;
	ym = (y1 + y2 + 0.5) / 2;

	bitmap_draw_line (buf, x1, y1, xm, ym);
	bitmap_draw_line (buf, x2, y2, xm, ym);
}


void bitmap_draw_box (struct buffer *buf,
	int x1, int y1,
	int x2, int y2)
{
	/* A box is just a series of four lines. */
	bitmap_draw_line (buf, x1, y1, x2, y1);
	bitmap_draw_line (buf, x1, y1, x1, y2);
	bitmap_draw_line (buf, x2, y1, x2, y2);
	bitmap_draw_line (buf, x1, y2, x2, y2);
}


void bitmap_draw_border (struct buffer *buf, unsigned int width)
{
	unsigned int i;

	/* For each pixel-width, draw one box. */
	for (i = 0; i < width; i++)
		bitmap_draw_box (buf, i, i, buf->width - i - 1, buf->height - i - 1);
}


struct coord zoom_out_translation (struct coord c)
{
	struct coord center = { .x = 64, .y = 16 };
	c.x = c.x + (center.x - c.x) * 0.2;
	c.y = c.y + (center.y - c.y) * 0.2;
	return c;
}


struct buffer *bitmap_translate (struct buffer *buf, translate_operator op)
{
	struct buffer *res = buffer_alloc (buf->len);
	unsigned int x, y;

	res->width = buf->width;
	res->height = buf->height;
	for (x = 0; x < buf->width; x++)
		for (y = 0 ; y < buf->height; y++)
		{
			struct coord c = { .x = x, .y = y };
			c = op (c);

			res->color = buf->data[bitmap_pos (buf, x, y)];

			bitmap_draw_pixel (res, c.x, c.y);
		}
	return res;
}


void bitmap_fill (struct buffer *buf, U8 val)
{
	unsigned int x, y;
	buf->color = val;
	for (x = 0; x < buf->width; x++)
		for (y = 0; y < buf->height; y++)
			bitmap_draw_pixel (buf, x, y);
}


/**
 * A legacy function that decodes the older FIF format into
 * an uncompressed, joined bitmap.  This allows the imglib
 * functions to be used on existing images.
 */
struct buffer *fif_decode (struct buffer *buf, unsigned int plane)
{
	struct buffer *res = buffer_alloc (FRAME_BYTE_SIZE);
	U8 *in = buf->data;
	U8 *out = res->data;
	U8 planes, method;
	U8 val, loop;
	unsigned int i;
	unsigned int bytes;

	planes = *in++;
	for (i = 0; i < planes; i++)
	{
		method = *in++;
		bytes = 0;
		switch (method)
		{
			case 1: /* RLE */
				while (bytes < FRAME_BYTE_SIZE)
				{
					val = *in++;
					if (val == 0xEE) /* skip */
					{
						loop = *in++;
						while (loop > 0)
						{
							if (i == plane)
								*out++ = 0;
							bytes++;
							loop--;
						}
					}
					else if (val == 0xED) /* repeat */
					{
						val = *in++;
						loop = *in++;
						while (loop > 0)
						{
							if (i == plane)
								*out++ = val;
							bytes++;
							loop--;
						}
					}
					else
					{
						if (i == plane)
							*out++ = val;
						bytes++;
					}
				}
				break;
			default:
				fprintf (stderr, "error: invalid FIF method\n");
				exit (1);
		}
	}

	res->len = out - res->data;
	return res;
}

struct buffer *binary_fif_read (const char *filename)
{
	FILE *fp;
	struct buffer *buf;

	buf = buffer_alloc (540);
	fp = fopen (filename, "rb");
	buffer_read (buf, fp);
	printf ("FIF:\n");
	buffer_write_c (buf, stdout);

	printf ("Decoded:\n");
	buf = buffer_replace (buf, fif_decode (buf, 1));
	buffer_write_c (buf, stdout);

	buf = buffer_replace (buf, buffer_splitbits (buf));
	buf->width = 128;
	buf->height = 32;
	fclose (fp);
	return buf;
}


void bitmap_finish (struct buffer *buf)
{
	buf = buffer_replace (buf, buffer_joinbits (buf));
	histogram_update (buf);
	buffer_write_c (buf, stdout);
	buffer_free (buf);
}


void bitmap_zoom_out (struct buffer *buf)
{
	for (;;)
	{
		buf = buffer_replace (buf, bitmap_translate (buf, zoom_out_translation));
		printf ("\n");
		bitmap_write_ascii (buf, stdout);
		getchar ();
	}
}


void layer_update (struct layer *layer)
{
	bitmap_fill (layer->buf, 0);
}

