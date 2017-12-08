# -*- coding: utf-8 -*-
#-------------------------------------------------------------------------#
#   Copyright (C) 2011 by Christoph Thelen                                #
#   doc_bacardi@users.sourceforge.net                                     #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
#-------------------------------------------------------------------------#


import os.path


#----------------------------------------------------------------------------
#
# Set up the Muhkuh Build System.
#
SConscript('mbs/SConscript')
Import('atEnv')

# Create a build environment for the ARM9 based netX chips.
env_arm9 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.7', 'asciidoc'])
env_arm9.CreateCompilerEnv('NETX500', ['arch=armv5te'])
env_arm9.CreateCompilerEnv('NETX56', ['arch=armv5te'])
env_arm9.CreateCompilerEnv('NETX50', ['arch=armv5te'])
env_arm9.CreateCompilerEnv('NETX10', ['arch=armv5te'])

# Create a build environment for the Cortex-R7 and Cortex-A9 based netX chips.
env_cortexR7 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc'])
env_cortexR7.CreateCompilerEnv('NETX4000_RELAXED', ['arch=armv7', 'thumb'], ['arch=armv7-r', 'thumb'])

# Create a build environment for the Cortex-M4 based netX chips.
env_cortexM4 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc'])
env_cortexM4.CreateCompilerEnv('NETX90_MPW', ['arch=armv7', 'thumb'], ['arch=armv7e-m', 'thumb'])
env_cortexM4.CreateCompilerEnv('NETX90_MPW_APP', ['arch=armv7', 'thumb'], ['arch=armv7e-m', 'thumb'])

# Build the platform libraries.
SConscript('platform/SConscript')


#----------------------------------------------------------------------------
#
# Get the source code version from the VCS.
#
atEnv.DEFAULT.Version('/tmp/targets/version/version.h', 'templates/version.h')


#----------------------------------------------------------------------------
# This is the list of sources. The elements must be separated with whitespace
# (i.e. spaces, tabs, newlines). The amount of whitespace does not matter.

src_standalone = """
src/init_standalone.S
src/main_standalone.c
src/progress_bar.c
src/alive_blinking.c
"""


#----------------------------------------------------------------------------
#
# Build all files.
#

# The list of include folders. Here it is used for all files.
astrIncludePaths = ['src', '#platform/src', '#platform/src/lib', '/tmp/targets/version', 'flasher_lib/includes']


atBuildConfigurations = {
    'netX56_standalone_flasher_intram': {
        'ENV': atEnv.NETX56,
        'LD': 'src/netx56/netx56_intram.ld',
        'FLASHER_LIB': 'flasher_lib/libflasher_netx56.a',
        'SRC': src_standalone,
        'DEFINES': [['CFG_DEBUGMSG', '0']],
        'BOOTIMAGES': {
            'netx56_standalone_flasher_intram.bin': {'SRC': 'SPI_GEN_25', 'DST': 'INTRAM' }
        },
        'FILTER': {},
    },
    'netX56_standalone_flasher_intram_dbg': {
        'ENV': atEnv.NETX56,
        'LD': 'src/netx56/netx56_intram.ld',
        'FLASHER_LIB': 'flasher_lib/libflasher_netx56_debug.a',
        'SRC': src_standalone,
        'DEFINES': [['CFG_DEBUGMSG', '1']],
        'BOOTIMAGES': {
            'netx56_standalone_flasher_intram_dbg.bin': {'SRC': 'SPI_GEN_25', 'DST': 'INTRAM' }
        },
        'FILTER': {},
    },
	
    'netX56_standalone_flasher_sdram': {
        'ENV': atEnv.NETX56,
        'LD': 'src/netx56/netx56_sdram.ld',
        'FLASHER_LIB': 'flasher_lib/libflasher_netx56.a',
        'SRC': src_standalone,
        'DEFINES': [['CFG_DEBUGMSG', '0']],
        'BOOTIMAGES': {
            'netx56_standalone_flasher_sdram.bin': {'SRC': 'SPI_GEN_25', 'DST': 'SD_MT48LC2M32B2' }
        },
        'FILTER': {},
    },
    'netX56_standalone_flasher_sdram_dbg': {
        'ENV': atEnv.NETX56,
        'LD': 'src/netx56/netx56_sdram.ld',
        'FLASHER_LIB': 'flasher_lib/libflasher_netx56_debug.a',
        'SRC': src_standalone,
        'DEFINES': [['CFG_DEBUGMSG', '1']],
        'BOOTIMAGES': {
            'netx56_standalone_flasher_sdram_dbg.bin': {'SRC': 'SPI_GEN_25', 'DST': 'SD_MT48LC2M32B2' }
        },
        'FILTER': {},
    },
	
}

# Collect the build results in the following dicts.
atBin = {}
atBootImages = {}

# Build all variants.
for strBuildName, atBuildAttributes in atBuildConfigurations.iteritems():
    # Get a clean environment.
    tEnv = atBuildAttributes['ENV'].Clone()
    # Set the include paths.
    tEnv.Append(CPPPATH = astrIncludePaths)
    # Set the linker description file.
    tEnv.Replace(LDFILE = atBuildAttributes['LD'])
    # Set the defines.
    tEnv.Append(CPPDEFINES = atBuildAttributes['DEFINES'])
    # Get the flasher library.
    tFlasherLibrary = [File(atBuildAttributes['FLASHER_LIB'])]
    # Set the build path.
    tSrc = tEnv.SetBuildPath(os.path.join('targets', strBuildName, 'build'), 'src', atBuildAttributes['SRC'])
    # Compile all sources and link the libraries.
    tElf = tEnv.Elf(os.path.join('targets', strBuildName, strBuildName + '.elf'), tSrc + tFlasherLibrary + tEnv['PLATFORM_LIBRARY'])
    # Create a complete dump of the ELF file.
    tTxt = tEnv.ObjDump(os.path.join('targets', strBuildName, strBuildName + '.txt'), tElf, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
    # Create a binary from the ELF file.
    tBin = tEnv.ObjCopy(os.path.join('targets', strBuildName, strBuildName + '.bin'), tElf)
    # Store the build in the dict.
    atBin[strBuildName] = tBin

    # Create all boot images.
    for strImage, atBootimage in atBuildAttributes['BOOTIMAGES'].iteritems():
        tImg = tEnv.BootBlock(os.path.join('targets', strBuildName, 'bootimages', strImage), tElf, BOOTBLOCK_SRC=atBootimage['SRC'], BOOTBLOCK_DST=atBootimage['DST'])

    # Filter additional files.
    for strDst, strSrc in atBuildAttributes['FILTER'].iteritems():
        tEnv.GccSymbolTemplate(os.path.join('targets', strBuildName, strDst), tElf, GCCSYMBOLTEMPLATE_TEMPLATE=strSrc)

