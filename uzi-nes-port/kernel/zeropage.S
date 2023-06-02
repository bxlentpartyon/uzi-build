	.export _udata

.segment "DATA"

.struct u_data
	u_ptab		.word
	u_insys		.byte
	u_callno	.byte
	u_retloc	.word
	u_retval	.byte
	u_error		.byte
	u_sp		.word
	u_bc		.word
	u_argn		.byte
	u_argn1		.byte
	u_argn2		.byte
	u_argn3		.byte

	u_base		.word
	u_count		.byte
	u_offset	.word
	u_buf		.word

	u_gid		.byte
	u_euid		.byte
	u_egid		.byte
	u_mask		.byte
	u_time		.res 4
	u_files		.res 10

	u_cwd		.word
	u_break		.word

	u_ino		.word
	u_isp		.word

	u_cursig	.res 32
	u_name		.res 8

	u_utime		.res 4
	u_stime		.res 4
	u_cutime	.res 4
	u_cstime	.res 4
.endstruct

_udata:	.tag u_data
