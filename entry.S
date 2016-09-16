//  ibex - payload trampoline: create payload entrypoint (see linker script)
//
//  Copyright (c) 2010, 2015-2016 xerub
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


	.global	_start
	.type	_start, %function
_start:
//	adrp	x8, __bss_start__
//	add	x8, x8, :lo12:__bss_start__
	b	_main
	.size	_start, .-_start
