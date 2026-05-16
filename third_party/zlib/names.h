#ifndef ZLIB_NAMES_H_
#define ZLIB_NAMES_H_

#define Z_ACT_PREFIX_SET

#define _dist_code act_z__dist_code
#define _length_code act_z__length_code
#define _tr_align act_z__tr_align
#define _tr_flush_bits act_z__tr_flush_bits
#define _tr_flush_block act_z__tr_flush_block
#define _tr_init act_z__tr_init
#define _tr_stored_block act_z__tr_stored_block
#define _tr_tally act_z__tr_tally
#define adler32 act_z_adler32
#define adler32_combine act_z_adler32_combine
#define adler32_combine64 act_z_adler32_combine64
#define adler32_z act_z_adler32_z
#define compress act_z_compress
#define compress2 act_z_compress2
#define compressBound act_z_compressBound
#define crc32 act_z_crc32
#define crc32_combine act_z_crc32_combine
#define crc32_combine64 act_z_crc32_combine64
#define crc32_z act_z_crc32_z
#define deflate act_z_deflate
#define deflateBound act_z_deflateBound
#define deflateCopy act_z_deflateCopy
#define deflateEnd act_z_deflateEnd
#define deflateGetDictionary act_z_deflateGetDictionary
/* #undef deflateInit */
/* #undef deflateInit2 */
#define deflateInit2_ act_z_deflateInit2_
#define deflateInit_ act_z_deflateInit_
#define deflateParams act_z_deflateParams
#define deflatePending act_z_deflatePending
#define deflatePrime act_z_deflatePrime
#define deflateReset act_z_deflateReset
#define deflateResetKeep act_z_deflateResetKeep
#define deflateSetDictionary act_z_deflateSetDictionary
#define deflateSetHeader act_z_deflateSetHeader
#define deflateTune act_z_deflateTune
#define deflate_copyright act_z_deflate_copyright
#define get_crc_table act_z_get_crc_table
#define gz_error act_z_gz_error
#define gz_intmax act_z_gz_intmax
#define gz_strwinerror act_z_gz_strwinerror
#define gzbuffer act_z_gzbuffer
#define gzclearerr act_z_gzclearerr
#define gzclose act_z_gzclose
#define gzclose_r act_z_gzclose_r
#define gzclose_w act_z_gzclose_w
#define gzdirect act_z_gzdirect
#define gzdopen act_z_gzdopen
#define gzeof act_z_gzeof
#define gzerror act_z_gzerror
#define gzflush act_z_gzflush
#define gzfread act_z_gzfread
#define gzfwrite act_z_gzfwrite
#define gzgetc act_z_gzgetc
#define gzgetc_ act_z_gzgetc_
#define gzgets act_z_gzgets
#define gzoffset act_z_gzoffset
#define gzoffset64 act_z_gzoffset64
#define gzopen act_z_gzopen
#define gzopen64 act_z_gzopen64
#define gzopen_w act_z_gzopen_w
#define gzprintf act_z_gzprintf
#define gzputc act_z_gzputc
#define gzputs act_z_gzputs
#define gzread act_z_gzread
#define gzrewind act_z_gzrewind
#define gzseek act_z_gzseek
#define gzseek64 act_z_gzseek64
#define gzsetparams act_z_gzsetparams
#define gztell act_z_gztell
#define gztell64 act_z_gztell64
#define gzungetc act_z_gzungetc
#define gzvprintf act_z_gzvprintf
#define gzwrite act_z_gzwrite
#define inflate act_z_inflate
#define inflateBack act_z_inflateBack
#define inflateBackEnd act_z_inflateBackEnd
/* #undef inflateBackInit */
#define inflateBackInit_ act_z_inflateBackInit_
#define inflateCodesUsed act_z_inflateCodesUsed
#define inflateCopy act_z_inflateCopy
#define inflateEnd act_z_inflateEnd
#define inflateGetDictionary act_z_inflateGetDictionary
#define inflateGetHeader act_z_inflateGetHeader
/* #undef inflateInit */
/* #undef inflateInit2 */
#define inflateInit2_ act_z_inflateInit2_
#define inflateInit_ act_z_inflateInit_
#define inflateMark act_z_inflateMark
#define inflatePrime act_z_inflatePrime
#define inflateReset act_z_inflateReset
#define inflateReset2 act_z_inflateReset2
#define inflateResetKeep act_z_inflateResetKeep
#define inflateSetDictionary act_z_inflateSetDictionary
#define inflateSync act_z_inflateSync
#define inflateSyncPoint act_z_inflateSyncPoint
#define inflateUndermine act_z_inflateUndermine
#define inflateValidate act_z_inflateValidate
#define inflate_copyright act_z_inflate_copyright
#define inflate_fast act_z_inflate_fast
#define inflate_table act_z_inflate_table
#define uncompress act_z_uncompress
#define uncompress2 act_z_uncompress2
#define zError act_z_zError
#define zcalloc act_z_zcalloc
#define zcfree act_z_zcfree
#define zlibCompileFlags act_z_zlibCompileFlags
#define zlibVersion act_z_zlibVersion
/* #undef Byte */
#define Bytef act_z_Bytef
#define alloc_func act_z_alloc_func
#define charf act_z_charf
#define free_func act_z_free_func
#define gzFile act_z_gzFile
#define gz_header act_z_gz_header
#define gz_headerp act_z_gz_headerp
#define in_func act_z_in_func
#define intf act_z_intf
#define out_func act_z_out_func
#define uInt act_z_uInt
#define uIntf act_z_uIntf
#define uLong act_z_uLong
#define uLongf act_z_uLongf
#define voidp act_z_voidp
#define voidpc act_z_voidpc
#define voidpf act_z_voidpf
#define gz_header_s act_z_gz_header_s
#define internal_state act_z_internal_state
/* #undef z_off64_t */

/* An exported symbol that isn't handled by Z_PREFIX in zconf.h */
#define z_errmsg act_z_z_errmsg

/* Symbols added in simd.patch */
#define copy_with_crc act_z_copy_with_crc
#define crc_finalize act_z_crc_finalize
#define crc_fold_512to32 act_z_crc_fold_512to32
#define crc_fold_copy act_z_crc_fold_copy
#define crc_fold_init act_z_crc_fold_init
#define crc_reset act_z_crc_reset
#define fill_window_sse act_z_fill_window_sse
#define read_buf act_z_read_buf
#define x86_check_features act_z_x86_check_features
#define x86_cpu_enable_simd act_z_x86_cpu_enable_simd

#endif  /* ZLIB_NAMES_H_ */
