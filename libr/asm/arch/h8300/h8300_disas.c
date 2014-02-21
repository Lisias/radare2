#include <r_types.h>
#include <r_util.h>
#include "h8300_disas.h"

static const char *commands_4bit[] = {
	[H8300_MOV_4BIT_2] = "mov.b",
	[H8300_MOV_4BIT_3] = "mov.b",
	[H8300_ADD_4BIT] = "add.b",
	[H8300_ADDX_4BIT] = "addx",
	[H8300_CMP_4BIT] = "cmp",
	[H8300_SUBX_4BIT] = "subx",
	[H8300_OR_4BIT] = "or",
	[H8300_XOR_4BIT] = "xor",
	[H8300_AND_4BIT] = "and",
	[H8300_MOV_4BIT] = "mov.b"
};

static const char *commands[] = {
	[H8300_ANDC] = "andc",
	[H8300_ADDB_DIRECT] = "add.b",
	[H8300_ADDW_DIRECT] = "add.w",
	[H8300_ADDS] = "adds",
	[H8300_AND] = "and.b",
	[H8300_ADDX] = "addx",
	[H8300_SUBW] = "sub.w",
	[H8300_BILD_IMM2R8] = "bild",
	/*
	[H8300_BAND_REG_DIR] = "band",
	[H8300_BAND_REG_IND] = "band",
	[H8300_BAND_ABS_ADR] = "band",
	*/
	[H8300_BCLR_R2R8] = "bclr",
	[H8300_BCLR_IMM2R8] = "bclr",
	[H8300_BCLR_R2IND16] = "bclr",
	[H8300_BCLR_R2ABS8] = "bclr",

	[H8300_BIAND_IMM2R8] = "biand",
	[H8300_BIAND_IMM2IND16] = "biand",
	[H8300_BIAND_IMM2ABS8] = "biand",
	[H8300_BST] = "bst",
	[H8300_BTST] = "btst",
	[H8300_BTST_R2R8] = "btst",

	[H8300_BSR] = "bsr",
	[H8300_NOP] = "nop",
	[H8300_DAA] = "daa",
	[H8300_DAS] = "das",
	[H8300_DEC] = "dec",
	[H8300_INC] = "inc",
	[H8300_NOT_NEG] = "neg",
	[H8300_OR] = "or",
	[H8300_DIVXU] = "divxu",
	[H8300_MULXU] = "mulxu",
	[H8300_EEPMOV] = "eepmov",
	[H8300_JMP_1] = "jmp",
	[H8300_JMP_2] = "jmp",
	[H8300_JMP_3] = "jmp",
	[H8300_JSR_1] = "jsr",
	[H8300_JSR_2] = "jsr",
	[H8300_JSR_3] = "jsr",
	[H8300_ORC] = "orc",
	[H8300_ROTL] = "rotl",
	[H8300_ROTR] = "rotr",
	[H8300_RTE] = "rte",
	[H8300_RTS] = "rts",
	[H8300_SHL] = "shal",
	[H8300_SHR] = "shar",
	[H8300_SLEEP] = "sleep",
	[H8300_STC] = "stc",
	[H8300_SUB_1] = "sub.b",
	[H8300_SUBW] = "sub.w",
	[H8300_SUBS] = "subs",
	[H8300_SUBX] = "subx",
	[H8300_XOR] = "xor",
	[H8300_XORC] = "xorc",

	[H8300_LDC] = "ldc",
	[H8300_LDC_2] = "ldc",

	[H8300_MOV_1] = "mov.b",
	[H8300_MOV_2] = "mov.w",
	[H8300_MOV_IMM162R16] = "mov.w",
	[H8300_MOV_DISP162R16] = "mov.w",
	[H8300_MOV_INDINC162R16] = "mov.w",
	[H8300_MOV_ABS162R16] = "mov.w",
	[H8300_MOV_IND162R16] = "mov.w",

	[H8300_MOV_R82IND16] = "mov.b",
	[H8300_MOV_R82DISPR16] = "mov.b",
	[H8300_MOV_R82RDEC16] = "mov.b",
	[H8300_MOV_R82ABS16] = "mov.b",

	[H8300_BRA] = "bra",
	[H8300_BRN] = "brn",
	[H8300_BHI] = "bhi",
	[H8300_BLS] = "bls",
	[H8300_BCC] = "bcc",
	[H8300_BCS] = "bcs",
	[H8300_BNE] = "bne",
	[H8300_BEQ] = "beq",
	[H8300_BVC] = "bvc",
	[H8300_BVS] = "bvs",
	[H8300_BPL] = "bpl",
	[H8300_BMI] = "bmi",
	[H8300_BGE] = "bge",
	[H8300_BLT] = "blt",
	[H8300_BGT] = "bgt",
	[H8300_BLE] = "ble",

	[H8300_CMP_1] = "cmp.b",
	[H8300_CMP_2] = "cmp.w",
};

static int decode_eepmov(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 4;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);
	cmd->operands[0] = '\0';

	switch (bytes[0]) {
	case H8300_RTS:
	case H8300_RTE:
		ret = 2;
		break;
	}

	return ret;
}

static int decode_ldc(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	if (bytes[0] == H8300_LDC_2 || bytes[0] == H8300_XORC) {
		snprintf(cmd->operands, H8300_INSTR_MAXLEN,
				"#0x%x,ccr", bytes[1]);
	} else if (bytes[0] == H8300_LDC) {
		snprintf(cmd->operands, H8300_INSTR_MAXLEN,
				"r%u%c,ccr", bytes[1] & 0x7,
				bytes[1] & 0x8 ? 'l' : 'h');
	} else if (bytes[0] == H8300_STC) {
		snprintf(cmd->operands, H8300_INSTR_MAXLEN,
				"crr,r%u%c", bytes[1] & 0x7,
				bytes[1] & 0x8 ? 'l' : 'h');
	}

	return ret;
}

static int decode_r82r8(const ut8 *bytes, struct h8300_cmd *cmd)
{
	ut8 rn = (bytes[1] >> 4) & 0x7;
	ut8 rm = bytes[1] & 0x7;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "r%u,r%u", rn, rm);

	return 2;
}

static int decode_r162r16(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "r%u,r%u",
			bytes[1] >> 4,
			bytes[1] & 0x7);

	return ret;
}

static int decode_andc(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "#0x%x,crr", bytes[1]);

	return ret;
}

static int decode_adds(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;
	unsigned reg, val;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	reg = bytes[1] & 0x7;

	if (bytes[1] & 0x80) {
		val = 2;
	} else {
		val = 1;
	}

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "#%u,r%u", val, reg);

	return ret;
}

static int decode_bsr(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, ".%d",
			(st8)bytes[1]);

	return ret;
}

/* [opcode ] [ 0000 | 0 rd] [      imm    ] */
static int decode_imm162r16(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 4;
	ut16 imm;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	r_mem_copyendian((ut8*)&imm, bytes + 2, sizeof(ut16), !LIL_ENDIAN);
	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "#0x%x,r%u",
			imm, bytes[1] & 0x7);

	return ret;
}

/* [ opcode ] [ 0 rs | 0 rd ] [         disp    ] */
static int decode_disp162r16(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 4;
	ut16 disp;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	r_mem_copyendian((ut8*)&disp, bytes + 2,
			sizeof(ut16), !LIL_ENDIAN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN,
			"@(0x%x,r%u),r%u", disp,
			(bytes[1] >> 4) & 0x7, bytes[1] & 0x7);

	return ret;
}

static int decode_pop(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;
	ut8 tmp = bytes[1] >> 4;

	strncpy(cmd->instr, tmp == 0x7 ? "pop" : "push",
			H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN,
			"r%u", bytes[1] & 0x7);

	return ret;
}

/* [ opcode ] [ 0 r2 | 0 rd ] @rs+,@rd */
static int decode_indinc162r16(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;
	ut8 tmp = bytes[1] >> 4;

	if (bytes[0] == 0x6D && (tmp == 7 || tmp == 0xF))
		return decode_pop(bytes, cmd);

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "@r%u+,r%u",
			(bytes[1] >> 4) & 0x7, bytes[1] & 0x7);

	return ret;
}

/* [ opcode ] [ 0 rs | 0 rd ] */
static int decode_ind162r16(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	if (bytes[1] & 0x80) {
		snprintf(cmd->operands, H8300_INSTR_MAXLEN, "r%u,@r%u",
			bytes[1] & 0x7,
			(bytes[1] >> 4) & 0x7);
	} else {
		snprintf(cmd->operands, H8300_INSTR_MAXLEN, "@r%u,r%u",
			(bytes[1] >> 4) & 0x7,
			bytes[1] & 0x7);
	}

	return ret;
}

/* [ opcode ] [0 | IMM | rd ] */
static int decode_imm2r8(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "#0x%x:3,r%u%c",
			(bytes[1] >> 4) & 0x7, bytes[1] & 0x7,
			bytes[1] & 0x8 ? 'l' : 'h');

	return ret;
}

/* [opcode] [0 | rd | 0000] [opcode] [0|IMM|0000] */
static int decode_imm2ind16(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 4;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "#0x%x:3,@r%u",
			bytes[3] >> 4, bytes[2] >> 4);

	return ret;
}

/* [opcode] [   abs   ] [opcode] [0|IMM | 0000] */
static int decode_imm2abs8(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 4;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);


	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "#0x%x:3,@0x%x:8",
			bytes[3] >> 4, bytes[1]);

	return ret;

}

/* [opcode] [ rn  |  rd ] */
static int decode_r2r8(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "r%u%c,r%u%c",
			(bytes[1] >> 4) & 0x7,
			bytes[1] & 0x80 ? 'l' : 'h',
			bytes[1] & 0x7, bytes[1] & 0x8  ? 'l' : 'h');

	return ret;
}

/* [opcode] [0| rd | 0000] [opcode] [ rn | 0 ] */
static int decode_r2ind16(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 4;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "r%u%c,@r%u",
			(bytes[3] >> 4) & 0x7,
			bytes[3] & 0x80 ? 'l' : 'h',
			bytes[1] >> 4);

	return ret;
}

/* [opcode] [ abs ] [opcode] [ rn | 0000 ] */
static int decode_r2abs8(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 4;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "r%u%c,@0x%x:8",
			(bytes[3] >> 4),
			bytes[3] & 0x80 ? 'l' : 'h',
			bytes[1]);

	return ret;
}

static int decode_subs(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "#%u,r%u",
			bytes[1] >> 7, bytes[1] & 0x7);

	return ret;
}

/* [ opcode ] [ 0000 |  rd ] */
static int decode_daa(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;
	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "r%u%c",
			(bytes[1] >> 4) & 0x7,
			bytes[1] & 0x80 ? 'l' : 'h');

	return ret;
}

/* [ opcode ] [ rs | 0 rd] */
static int decode_r82r16(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "r%u%c,r%u",
			(bytes[1] >> 4) & 0x7,
			bytes[1] & 0x80 ? 'l' : 'h',
			bytes[1] & 0x7);

	return ret;
}

/* [opcode] [0000 0000] [       abs    ] */
int decode_jmp_abs16(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 4;
	ut16 abs;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	r_mem_copyendian((ut8*)&abs, bytes + 2, 2, !LIL_ENDIAN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "@0x%x", abs);

	return ret;
}

/* [opcode] [  abs    ] */
int decode_jmp_abs8(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN,
			"@@%x", bytes[1]);

	return ret;
}

/* [opcode] [0 rn 0000] */
static int decode_jmp_ind(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN,
			"@r%u", (bytes[0] >> 4) & 0x7);

	return ret;
}

/* [ opcode ] [ 0000 | 0 rd ] [     abs    ] */
static int decode_abs162r16(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 4;
	ut16 abs;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	r_mem_copyendian((ut8*)&abs, bytes + 2, sizeof(ut16), !LIL_ENDIAN);
	if (bytes[1] & 0x80) {
		snprintf(cmd->operands, H8300_INSTR_MAXLEN,
				"r%u,@0x%x", bytes[1] & 0x7, abs);
	} else {
		snprintf(cmd->operands, H8300_INSTR_MAXLEN, "@0x%x,r%u",
			abs, bytes[1] & 0x7);
	}

	return ret;
}

/* [ opcode ] [ 1 rd | rs ] */
static int decode_r82ind16(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "r%u%c,@r%u",
			bytes[1] & 0x7, bytes[1] & 0x8 ? 'l' : 'h',
			(bytes[1] >> 4) & 0x7);

	return ret;
}

/* [ opcode ] [ 1 rd |  rs ] [       disp     ] */
static int decode_r82dispr16(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 4;
	ut16 disp;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	r_mem_copyendian((ut8*)&disp, bytes + 2, sizeof(ut16), !LIL_ENDIAN);
	snprintf(cmd->operands, H8300_INSTR_MAXLEN,
			"r%u%c,@(0x%x,r%u)",
			bytes[1] & 0x7, bytes[1] & 0x8 ? 'l' : 'h',
			disp, (bytes[1] >> 4) & 0x7);

	return ret;
}

/* [ opcode ] [1 rd rs ] */
static int decode_r82rdec16(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN,
			"r%u%c,@-r%u",
			bytes[1] & 0x7, bytes[1] & 0x8 ? 'l' : 'h',
			(bytes[1] >> 4) & 0x7);
	return ret;
}

/* [opcode ] [ 8 | rs ] [    abs    ] */
static int decode_r82abs16(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 4;
	ut16 abs;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	r_mem_copyendian((ut8*)&abs, bytes + 2, sizeof(ut16), !LIL_ENDIAN);

	if (bytes[1] & 0x80) {
		snprintf(cmd->operands, H8300_INSTR_MAXLEN, "r%u,@0x%x",
			bytes[1] & 0x7, abs);
	} else {
		snprintf(cmd->operands, H8300_INSTR_MAXLEN, "@0x%x,r%u%c",
			abs, bytes[1] & 0x7,
			bytes[1] & 0x8 ? 'l' : 'h');
	}

	return ret;
}

static int decode_nop(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands[bytes[0]], H8300_INSTR_MAXLEN);

	cmd->operands[0] = '\0';

	return ret;
}

static int decode_abs2r_short(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands_4bit[bytes[0] >> 4], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN,
			"@0x%x:8,r%u%c",
				bytes[1], (bytes[0] >> 4) & 0x7,
				bytes[0] & 0x8 ? 'l' : 'h');

	return ret;
}

static int decode_r2imm_short(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands_4bit[bytes[0] >> 4], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN,
				"r%u%c,@0x%x:8",
				bytes[0] >> 4, bytes[0] & 0x8 ? 'l' : 'h',
				bytes[1]);
	return ret;
}

static int decode_imm2r_short(const ut8 *bytes, struct h8300_cmd *cmd)
{
	int ret = 2;

	strncpy(cmd->instr, commands_4bit[bytes[0] >> 4], H8300_INSTR_MAXLEN);

	snprintf(cmd->operands, H8300_INSTR_MAXLEN, "#0x%x:8,r%u%c",
			bytes[1], bytes[0] & 0x7, bytes[0] & 0x8 ? 'l' : 'h');

	return ret;
}

/*
static int decode_imm82r8_long(const ut8 *instr, struct h8300_cmd *cmd)
{
	strncpy(cmd->instr, commands_
}
*/

int h8300_decode_command(const ut8 *instr, struct h8300_cmd *cmd)
{
	int ret = 0;

	switch (instr[0] >> 4) {
	case H8300_MOV_4BIT_3:
		ret = decode_r2imm_short(instr, cmd);
		break;
	case H8300_MOV_4BIT_2:
		ret = decode_abs2r_short(instr, cmd);
		break;
	case H8300_AND_4BIT:
	case H8300_ADDX_4BIT:
	case H8300_ADD_4BIT:
	case H8300_CMP_4BIT:
	case H8300_MOV_4BIT:
	case H8300_OR_4BIT:
	case H8300_SUBX_4BIT:
	case H8300_XOR_4BIT:
		ret = decode_imm2r_short(instr, cmd);
		break;
	}

	if (ret)
		return ret;

	switch (instr[0]) {
	case H8300_ANDC:
		ret = decode_andc(instr, cmd);
		break;
	case H8300_SUBS:
		ret = decode_subs(instr, cmd);
		break;
	case H8300_ADDB_DIRECT:
	case H8300_ADDW_DIRECT:
	case H8300_ADDX:
	case H8300_AND:
	case H8300_CMP_2:
		ret = decode_r162r16(instr, cmd);
		break;
	case H8300_ADDS:
		ret = decode_adds(instr, cmd);
		break;
	case H8300_BIAND_IMM2R8:
	case H8300_BCLR_IMM2R8:
	case H8300_BST:
	case H8300_BTST:
	case H8300_BILD_IMM2R8:
		ret = decode_imm2r8(instr, cmd);
		break;
	case H8300_BCLR_R2R8:
	case H8300_SUB_1:
	case H8300_SUBX:
	case H8300_XOR:
	case H8300_BTST_R2R8:
		ret = decode_r2r8(instr, cmd);
		break;
	case H8300_BCLR_R2IND16:
		ret = decode_r2ind16(instr, cmd);
		break;
	case H8300_BCLR_R2ABS8:
		ret = decode_r2abs8(instr, cmd);
		break;
	case H8300_BIAND_IMM2IND16:
		ret = decode_imm2ind16(instr, cmd);
		break;
	case H8300_BIAND_IMM2ABS8:
		ret = decode_imm2abs8(instr, cmd);
		break;
	case H8300_BSR:
		ret = decode_bsr(instr, cmd);
		break;
	case H8300_NOP:
		ret = decode_nop(instr, cmd);
		break;
	case H8300_DAA:
	case H8300_DAS:
	case H8300_DEC:
	case H8300_INC:
	case H8300_NOT_NEG:
	case H8300_ROTL:
	case H8300_ROTR:
	case H8300_SHL:
	case H8300_SHR:
		ret = decode_daa(instr, cmd);
		break;
	case H8300_DIVXU:
	case H8300_MULXU:
		ret = decode_r82r16(instr, cmd);
		break;
	case H8300_EEPMOV:
	case H8300_RTS:
	case H8300_RTE:
	case H8300_SLEEP:
		ret = decode_eepmov(instr, cmd);
		break;
	case H8300_JMP_1:
	case H8300_JSR_1:
		ret = decode_jmp_ind(instr, cmd);
		break;
	case H8300_JMP_2:
	case H8300_JSR_2:
		ret = decode_jmp_abs16(instr, cmd);
		break;
	case H8300_JMP_3:
	case H8300_JSR_3:
	case H8300_BRA:
	case H8300_BRN:
	case H8300_BHI:
	case H8300_BLS:
	case H8300_BCC:
	case H8300_BCS:
	case H8300_BNE:
	case H8300_BEQ:
	case H8300_BVC:
	case H8300_BVS:
	case H8300_BPL:
	case H8300_BMI:
	case H8300_BGE:
	case H8300_BLT:
	case H8300_BGT:
	case H8300_BLE:
		ret = decode_jmp_abs8(instr, cmd);
		break;
	case H8300_ORC:
	case H8300_LDC:
	case H8300_LDC_2:
	case H8300_STC:
		ret = decode_ldc(instr, cmd);
		break;
	case H8300_MOV_1:
	case H8300_OR:
		ret = decode_r82r8(instr, cmd);
		break;
	case H8300_MOV_2:
	case H8300_SUBW:
		ret = decode_r162r16(instr, cmd);
		break;
	case H8300_MOV_IMM162R16:
		ret = decode_imm162r16(instr, cmd);
		break;
	case H8300_MOV_IND162R16:
		ret = decode_ind162r16(instr, cmd);
		break;
	case H8300_MOV_DISP162R16:
		ret = decode_disp162r16(instr, cmd);
		break;
	case H8300_MOV_INDINC162R16:
		ret = decode_indinc162r16(instr, cmd);
		break;
	case H8300_MOV_ABS162R16:
		ret = decode_abs162r16(instr, cmd);
		break;
	case H8300_MOV_R82IND16:
		ret = decode_r82ind16(instr, cmd);
		break;
	case H8300_MOV_R82DISPR16:
		ret = decode_r82dispr16(instr, cmd);
		break;
	case H8300_MOV_R82RDEC16:
		ret = decode_r82rdec16(instr, cmd);
		break;
	case H8300_MOV_R82ABS16:
		ret = decode_r82abs16(instr, cmd);
		break;
	default:
		return -1;
	}

	return ret;
}
