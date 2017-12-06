# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------- #
#   Copyright (C) 2017 by Christoph Thelen                                #
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
# ----------------------------------------------------------------------- #

import argparse
import array
import logging

__revision__ = '0.0.2'


def concat_and_patch_image(strExistingImageFileName, strDataFileName, strOutputFileName, ulGeneralCtrl, ulTimingCtrl):
    # Read the complete image into 2 strings.
    # The first 64 bytes are the header.
    # The rest of the file is the application data.
    logging.info('Reading the image file "%s".' % strExistingImageFileName)
    tBinFile = open(strExistingImageFileName, 'rb')
    strHeader = tBinFile.read(64)
    strApp = tBinFile.read()
    tBinFile.close()

    # Convert the strings to arrays.
    aBootBlock = array.array('I', strHeader)
    aulApplicationData = array.array('I', strApp)

    # Read the complete data file.
    logging.info('Reading the data file "%s".' % strDataFileName)
    tBinFile = open(strDataFileName, 'rb')
    strData = tBinFile.read()
    tBinFile.close()

    # Append the data to the application data.
    aulData = array.array('I', strData)
    aulApplicationData.extend(aulData)

    # Test if the header starts with the magic cookie.
    if aBootBlock[0x00] != 0xf8beaf00:
        # This is an unknown image.
        raise Exception('The input file does not have a valid magic cookie!')

    # Build the application checksum.
    logging.info('Building the application checksum.')
    ulApplicationChecksum = 0
    for ulData in aulApplicationData:
        ulApplicationChecksum += ulData
        ulApplicationChecksum &= 0xffffffff
    logging.debug('The new application checksum is 0x%08x.' % ulApplicationChecksum)

    # Get the application size in 32bit chunks.
    ulApplicationDwordSize = len(aulApplicationData)

    # Set the application checksum and size in the header.
    aBootBlock[0x03] = ulApplicationChecksum
    aBootBlock[0x04] = ulApplicationDwordSize

    # Are there SDRAM parameters?
    if (ulGeneralCtrl is not None) and (ulTimingCtrl is not None):
        # Write the SDRAM parameters to the header.
        logging.info('Write the SDRAM parameter to the header.')
        logging.debug('SDRAM general ctrl = 0x%08x' % ulGeneralCtrl)
        logging.debug('SDRAM timing ctrl = 0x%08x' % ulTimingCtrl)
        aBootBlock[0x07] = ulGeneralCtrl
        aBootBlock[0x08] = ulTimingCtrl

    # Build the bootblock checksum.
    logging.info('Building the new header checksum.')
    ulBootblockChecksum = 0
    for uiCnt in range(0, 15):
        ulBootblockChecksum += aBootBlock[uiCnt]
        ulBootblockChecksum &= 0xffffffff
    ulBootblockChecksum = (ulBootblockChecksum - 1) ^ 0xffffffff
    aBootBlock[0x0f] = ulBootblockChecksum
    logging.debug('The new header checksum is 0x%08x.' % ulBootblockChecksum)

    # Write the bootimage.
    logging.info('Writing the new boot image to the file "%s".' % strOutputFileName,)
    fOutput = open(strOutputFileName, 'wb')
    aBootBlock.tofile(fOutput)
    aulApplicationData.tofile(fOutput)
    fOutput.close()

    logging.info('Finished.')


atLogLevels = {
    'critical': logging.CRITICAL,
    'error': logging.ERROR,
    'warning': logging.WARNING,
    'info': logging.INFO,
    'debug': logging.DEBUG
}

tParser = argparse.ArgumentParser()
tParser.add_argument('strExistingImage',
                     metavar='IMAGE_FILE',
                     help='Read the existing image from IMAGE_FILE.')
tParser.add_argument('strDataFile',
                     metavar='DATA_FILE',
                     help='Read the data to appent to the existing image from DATA_FILE.')
tParser.add_argument('strOutputFile',
                     metavar='OUTPUT_FILE',
                     help='Write the resulting boot image to OUTPUT_FILE.')
tParser.add_argument('-g', '--general-ctrl',
                     dest='ulGeneralCtrl',
                     required=False,
                     default=None,
                     metavar='GENERAL_CTRL_VALUE',
                     help='Configure an SDRAM with the general control register set to GENERAL_CTRL_VALUE',
                     type=int)
tParser.add_argument('-t', '--timing-ctrl',
                     dest='ulTimingCtrl',
                     required=False,
                     default=None,
                     metavar='TIMING_CTRL_VALUE',
                     help='Configure an SDRAM with the timing control register set to TIMING_CTRL_VALUE',
                     type=int)
tParser.add_argument('-v', '--verbose',
                     dest='tVerboseLevel',
                     required=False,
                     default='info',
                     choices=atLogLevels.keys(),
                     metavar='LEVEL',
                     help='Set the log level to LEVEL. Possible values for LEVEL are %s' % ', '.join(atLogLevels.keys()))
tArgs = tParser.parse_args()

# Create the logger object.
tLogLevel = atLogLevels[tArgs.tVerboseLevel]
logging.basicConfig(format='%(asctime)-15s [%(levelname)s]: %(message)s', level=tLogLevel)
logging.info('append v%s by cthelen@hilscher.com' % __revision__)

if (tArgs.ulGeneralCtrl is not None) and (tArgs.ulTimingCtrl is None):
    logging.warning('The general control register is set, but not the timing control register. Ignoring SDRAM parameter.')
if (tArgs.ulGeneralCtrl is None) and (tArgs.ulTimingCtrl is not None):
    logging.warning('The timing control register is set, but not the general control register. Ignoring SDRAM parameter.')

concat_and_patch_image(tArgs.strExistingImage, tArgs.strDataFile, tArgs.strOutputFile, tArgs.ulGeneralCtrl, tArgs.ulTimingCtrl)
