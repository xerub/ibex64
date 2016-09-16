//  ibex - printf shim: GNU AArch64 ABI -> Apple ARM64 ABI (max 7 args)
//
//  Copyright (c) 2016 xerub
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#define fp x29
#define lr x30

	.global	_printf
	.type	_printf, %function
_printf:
	stp	fp, lr, [sp, #-16]!
	mov	fp, sp

	stp	x7, x8, [sp, #-16]!
	stp	x5, x6, [sp, #-16]!
	stp	x3, x4, [sp, #-16]!
	stp	x1, x2, [sp, #-16]!

	mov	x8, x0
	bl	get_printf_addr		// XXX hopefully x8 will not get clobbered
	mov	x1, x0
	mov	x0, x8
	blr	x1

	ldp	x1, x2, [sp], #16
	ldp	x3, x4, [sp], #16
	ldp	x5, x6, [sp], #16
	ldp	x7, x8, [sp], #16

	ldp	fp, lr, [sp], #16
	ret
	.size	_printf, .-_printf
