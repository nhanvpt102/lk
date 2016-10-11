/************
 *
 * Filename:  sierra_cwe.c
 *
 * Purpose:   Sierra Little Kernel changes            
 *
 * Copyright: (c) 2015 Sierra Wireless, Inc.
 *            All rights reserved
 *
 * Note:       
 *
 ************/

#include <string.h>
#include <reg.h>
#include <debug.h>
#include <platform.h>
#include <platform/iomap.h>
#include <arch/ops.h>
#include <arch/arm/mmu.h>

#include "sierra_cweudefs.h"

/*
 *  externs
 */


/*
 *  Local variables
 */
 
/* Local constants and enumerated types */

/* This table was generated by the "crctable" program */
_local const uint32 crc32table[256] =
{
  0x00000000U, 0x77073096U, 0xEE0E612CU, 0x990951BAU,     /* 0x00 */
  0x076DC419U, 0x706AF48FU, 0xE963A535U, 0x9E6495A3U,     /* 0x04 */
  0x0EDB8832U, 0x79DCB8A4U, 0xE0D5E91EU, 0x97D2D988U,     /* 0x08 */
  0x09B64C2BU, 0x7EB17CBDU, 0xE7B82D07U, 0x90BF1D91U,     /* 0x0C */
  0x1DB71064U, 0x6AB020F2U, 0xF3B97148U, 0x84BE41DEU,     /* 0x10 */
  0x1ADAD47DU, 0x6DDDE4EBU, 0xF4D4B551U, 0x83D385C7U,     /* 0x14 */
  0x136C9856U, 0x646BA8C0U, 0xFD62F97AU, 0x8A65C9ECU,     /* 0x18 */
  0x14015C4FU, 0x63066CD9U, 0xFA0F3D63U, 0x8D080DF5U,     /* 0x1C */
  0x3B6E20C8U, 0x4C69105EU, 0xD56041E4U, 0xA2677172U,     /* 0x20 */
  0x3C03E4D1U, 0x4B04D447U, 0xD20D85FDU, 0xA50AB56BU,     /* 0x24 */
  0x35B5A8FAU, 0x42B2986CU, 0xDBBBC9D6U, 0xACBCF940U,     /* 0x28 */
  0x32D86CE3U, 0x45DF5C75U, 0xDCD60DCFU, 0xABD13D59U,     /* 0x2C */
  0x26D930ACU, 0x51DE003AU, 0xC8D75180U, 0xBFD06116U,     /* 0x30 */
  0x21B4F4B5U, 0x56B3C423U, 0xCFBA9599U, 0xB8BDA50FU,     /* 0x34 */
  0x2802B89EU, 0x5F058808U, 0xC60CD9B2U, 0xB10BE924U,     /* 0x38 */
  0x2F6F7C87U, 0x58684C11U, 0xC1611DABU, 0xB6662D3DU,     /* 0x3C */
  0x76DC4190U, 0x01DB7106U, 0x98D220BCU, 0xEFD5102AU,     /* 0x40 */
  0x71B18589U, 0x06B6B51FU, 0x9FBFE4A5U, 0xE8B8D433U,     /* 0x44 */
  0x7807C9A2U, 0x0F00F934U, 0x9609A88EU, 0xE10E9818U,     /* 0x48 */
  0x7F6A0DBBU, 0x086D3D2DU, 0x91646C97U, 0xE6635C01U,     /* 0x4C */
  0x6B6B51F4U, 0x1C6C6162U, 0x856530D8U, 0xF262004EU,     /* 0x50 */
  0x6C0695EDU, 0x1B01A57BU, 0x8208F4C1U, 0xF50FC457U,     /* 0x54 */
  0x65B0D9C6U, 0x12B7E950U, 0x8BBEB8EAU, 0xFCB9887CU,     /* 0x58 */
  0x62DD1DDFU, 0x15DA2D49U, 0x8CD37CF3U, 0xFBD44C65U,     /* 0x5C */
  0x4DB26158U, 0x3AB551CEU, 0xA3BC0074U, 0xD4BB30E2U,     /* 0x60 */
  0x4ADFA541U, 0x3DD895D7U, 0xA4D1C46DU, 0xD3D6F4FBU,     /* 0x64 */
  0x4369E96AU, 0x346ED9FCU, 0xAD678846U, 0xDA60B8D0U,     /* 0x68 */
  0x44042D73U, 0x33031DE5U, 0xAA0A4C5FU, 0xDD0D7CC9U,     /* 0x6C */
  0x5005713CU, 0x270241AAU, 0xBE0B1010U, 0xC90C2086U,     /* 0x70 */
  0x5768B525U, 0x206F85B3U, 0xB966D409U, 0xCE61E49FU,     /* 0x74 */
  0x5EDEF90EU, 0x29D9C998U, 0xB0D09822U, 0xC7D7A8B4U,     /* 0x78 */
  0x59B33D17U, 0x2EB40D81U, 0xB7BD5C3BU, 0xC0BA6CADU,     /* 0x7C */
  0xEDB88320U, 0x9ABFB3B6U, 0x03B6E20CU, 0x74B1D29AU,     /* 0x80 */
  0xEAD54739U, 0x9DD277AFU, 0x04DB2615U, 0x73DC1683U,     /* 0x84 */
  0xE3630B12U, 0x94643B84U, 0x0D6D6A3EU, 0x7A6A5AA8U,     /* 0x88 */
  0xE40ECF0BU, 0x9309FF9DU, 0x0A00AE27U, 0x7D079EB1U,     /* 0x8C */
  0xF00F9344U, 0x8708A3D2U, 0x1E01F268U, 0x6906C2FEU,     /* 0x90 */
  0xF762575DU, 0x806567CBU, 0x196C3671U, 0x6E6B06E7U,     /* 0x94 */
  0xFED41B76U, 0x89D32BE0U, 0x10DA7A5AU, 0x67DD4ACCU,     /* 0x98 */
  0xF9B9DF6FU, 0x8EBEEFF9U, 0x17B7BE43U, 0x60B08ED5U,     /* 0x9C */
  0xD6D6A3E8U, 0xA1D1937EU, 0x38D8C2C4U, 0x4FDFF252U,     /* 0xA0 */
  0xD1BB67F1U, 0xA6BC5767U, 0x3FB506DDU, 0x48B2364BU,     /* 0xA4 */
  0xD80D2BDAU, 0xAF0A1B4CU, 0x36034AF6U, 0x41047A60U,     /* 0xA8 */
  0xDF60EFC3U, 0xA867DF55U, 0x316E8EEFU, 0x4669BE79U,     /* 0xAC */
  0xCB61B38CU, 0xBC66831AU, 0x256FD2A0U, 0x5268E236U,     /* 0xB0 */
  0xCC0C7795U, 0xBB0B4703U, 0x220216B9U, 0x5505262FU,     /* 0xB4 */
  0xC5BA3BBEU, 0xB2BD0B28U, 0x2BB45A92U, 0x5CB36A04U,     /* 0xB8 */
  0xC2D7FFA7U, 0xB5D0CF31U, 0x2CD99E8BU, 0x5BDEAE1DU,     /* 0xBC */
  0x9B64C2B0U, 0xEC63F226U, 0x756AA39CU, 0x026D930AU,     /* 0xC0 */
  0x9C0906A9U, 0xEB0E363FU, 0x72076785U, 0x05005713U,     /* 0xC4 */
  0x95BF4A82U, 0xE2B87A14U, 0x7BB12BAEU, 0x0CB61B38U,     /* 0xC8 */
  0x92D28E9BU, 0xE5D5BE0DU, 0x7CDCEFB7U, 0x0BDBDF21U,     /* 0xCC */
  0x86D3D2D4U, 0xF1D4E242U, 0x68DDB3F8U, 0x1FDA836EU,     /* 0xD0 */
  0x81BE16CDU, 0xF6B9265BU, 0x6FB077E1U, 0x18B74777U,     /* 0xD4 */
  0x88085AE6U, 0xFF0F6A70U, 0x66063BCAU, 0x11010B5CU,     /* 0xD8 */
  0x8F659EFFU, 0xF862AE69U, 0x616BFFD3U, 0x166CCF45U,     /* 0xDC */
  0xA00AE278U, 0xD70DD2EEU, 0x4E048354U, 0x3903B3C2U,     /* 0xE0 */
  0xA7672661U, 0xD06016F7U, 0x4969474DU, 0x3E6E77DBU,     /* 0xE4 */
  0xAED16A4AU, 0xD9D65ADCU, 0x40DF0B66U, 0x37D83BF0U,     /* 0xE8 */
  0xA9BCAE53U, 0xDEBB9EC5U, 0x47B2CF7FU, 0x30B5FFE9U,     /* 0xEC */
  0xBDBDF21CU, 0xCABAC28AU, 0x53B39330U, 0x24B4A3A6U,     /* 0xF0 */
  0xBAD03605U, 0xCDD70693U, 0x54DE5729U, 0x23D967BFU,     /* 0xF4 */
  0xB3667A2EU, 0xC4614AB8U, 0x5D681B02U, 0x2A6F2B94U,     /* 0xF8 */
  0xB40BBE37U, 0xC30C8EA1U, 0x5A05DF1BU, 0x2D02EF8DU      /* 0xFC */
};

/* The order of entries in this table must match the order of the enums in
 * cwe_image_type_e */
_local const char cwe_image_str[CWE_IMAGE_TYPE_COUNT][sizeof(uint32)] = {
  {'Q', 'P', 'A', 'R'},
  {'S', 'B', 'L', '1'},
  {'S', 'B', 'L', '2'},
  {'D', 'S', 'P', '1'},
  {'D', 'S', 'P', '2'},
  {'D', 'S', 'P', '3'},
  {'Q', 'R', 'P', 'M'},
  {'B', 'O', 'O', 'T'},
  {'A', 'P', 'P', 'L'},
  {'O', 'S', 'B', 'L'},
  {'A', 'M', 'S', 'S'},
  {'A', 'P', 'P', 'S'},
  {'A', 'P', 'B', 'L'},
  {'N', 'V', 'B', 'F'},
  {'N', 'V', 'B', 'O'},
  {'N', 'V', 'B', 'U'},
  {'E', 'X', 'E', 'C'},
  {'S', 'W', 'O', 'C'},
  {'F', 'O', 'T', 'O'},
  {'F', 'I', 'L', 'E'},
  {'S', 'P', 'K', 'G'},
  {'M', 'O', 'D', 'M'},
  {'S', 'Y', 'S', 'T'},
  {'U', 'S', 'E', 'R'},
  {'H', 'D', 'A', 'T'},
  {'N', 'V', 'B', 'C'},
  {'S', 'P', 'L', 'A'},
  {'N', 'V', 'U', 'P'},
  {'Q', 'M', 'B', 'A'},
  {'T', 'Z', 'O', 'N'},
  {'Q', 'S', 'D', 'I'},
  {'A', 'R', 'C', 'H'},
  {'U', 'A', 'P', 'P'},
  {'L', 'R', 'A', 'M'},
};

/* 
 * functions 
 */

/*************
 *
 * Name:     crcrc32 - CRC-32 calculator
 *
 * Purpose:  To calculate the CRC-32 of a buffer
 *
 * Parms:    (IN) address - input buffer
 *           size         - number of bytes to read
 *           crc          - starting CRC seed
 *
 * Return:   32-bit CRC
 *
 * Abort:    None
 *
 * Notes:    None
 *
 **************/
_global uint32 crcrc32(uint8 *address, uint32 size, uint32 crc)
{
  while (size-- != 0)
  {
    /* byte loop */
    crc = (((crc >> 8) & 0x00FFFFFFU) ^ crc32table[(crc ^ (uint32) (*address++)) & 0x000000FFU]);
  }
  return(crc);
}


/*************
 *
 * Name:      piget32
 *
 * Purpose:   To get a 32 bit value from a packet in network byte
 *            order and increment the packet pointer beyond the
 *            extracted field
 *
 * Parms:     packetpp - memory location of the pointer to the packet
 *                       from which the 32 bit value will be read
 *
 * Return:    32 bit value
 *
 * Abort:     none
 *
 * Notes:     This function performs no pointer validation
 *
 **************/
_global uint32 piget32 (
  uint8 ** packetpp)
{
  uint32 field;
  uint8 *packetp;

  packetp = *packetpp;

  field = *packetp++;
  field <<= 8;
  field += *packetp++;
  field <<= 8;
  field += *packetp++;
  field <<= 8;
  field += *packetp++;

  *packetpp = packetp;

  return (field);
}

/*************
 *
 * Name:      pigetmulti
 *
 * Purpose:   To get a string of 8-bit fields from a packet and
 *            increment the packet pointer beyond the last read 8-bit
 *            field
 *
 * Parms:     packetpp  - memory location of a pointer to a packet
 *                        from which the string of 8-bit fields is to
 *                        be read
 *
 *            bufferp   - pointer to a buffer into which the 8-bit
 *                        fields are to be copied
 *
 *            numfields - number of 8-bit fields to be copied
 *
 * Return:    none
 *
 * Abort:     none
 *
 * Notes:     This function performs no pointer validation
 *
 **************/
_global void pigetmulti (
  uint8 ** packetpp,
  uint8 * bufferp,
  uint16 numfields)
{
  uint8 *packetp;
  int32 i;

  packetp = *packetpp;

  for (i = numfields - 1; i >= 0; i--)
  {
    *bufferp++ = *packetp++;
  }

  *packetpp = packetp;
}

/************
 *
 * Name:     cwe_image_string_get
 *
 * Purpose:  Returns the string for a specified CWE image type
 *
 * Parms:    (IN) imagetype - Desired CWE Image Type
 *
 * Return:   Pointer to image type string if valid image
 *
 * Abort:    None
 *
 * Notes:    None
 *
 ************/
_global const char *cwe_image_string_get(
  enum cwe_image_type_e imagetype)
{
  if ((imagetype < CWE_IMAGE_TYPE_COUNT))
  {
    return (cwe_image_str[imagetype]);
  }
  return ((const char *)NULL);
}

/************
 *
 * Name:     cwe_image_value_get
 *
 * Purpose:  Returns an integer value for the specified CWE image type
 *
 * Parms:    (IN) imagetype - CWE Image Type to convert
 *
 * Return:   Integer value for a valid image type or CWE_IMAGE_TYPE_MAX
 *           if image type is invalid
 *
 * Abort:    None
 *
 * Notes:    None
 *
 ************/
_global uint32 cwe_image_value_get(
  enum cwe_image_type_e imagetype)
{
  uint32 imageval = CWE_IMAGE_TYPE_INVALID;

  if ((imagetype < CWE_IMAGE_TYPE_COUNT))
  {
    imageval = (cwe_image_str[imagetype][0] << 24) |
               (cwe_image_str[imagetype][1] << 16) |
               (cwe_image_str[imagetype][2] << 8)  | 
                cwe_image_str[imagetype][3];
  }

  return imageval;
}

/************
 *
 * Name:     cwe_image_type_validate
 *
 * Purpose:  Validates the image type against supported values
 *
 * Parms:    (IN)  imagetype - image type for validatation
 *           (OUT) enumvalue - enum value for image type
 *
 * Return:   TRUE if image type is one of the supported values
 *           FALSE otherwise
 *
 * Abort:    None
 *
 * Notes:    enumvalue = CWE_IMAGE_TYPE_MAX if imagetype is not valid
 *
 ************/
_global boolean cwe_image_type_validate(
  uint32 imagetype,
  enum cwe_image_type_e * enumvalue)
{
  boolean retVal = TRUE;        /* default return value */
  enum cwe_image_type_e idx;
  uint32 imageval;

  if (enumvalue == NULL)
  {
    return FALSE;
  }

  for (idx = CWE_IMAGE_TYPE_MIN; idx < CWE_IMAGE_TYPE_COUNT; idx++)
  {
    imageval = cwe_image_value_get(idx);
    if (imageval == imagetype)
    {
      break;
    }
  }

  *enumvalue = idx;             /* save found index */

  if (idx == CWE_IMAGE_TYPE_COUNT)        /* imagetype not found */
  {
    retVal = FALSE;
  }

  return retVal;
}


/************
 *
 * Name:     cwe_header_load
 *
 * Purpose:  Read the CWE header and store the fields
 *
 * Params:   (IN)  startp - start address of the CWE header to be read
 *           (OUT) hdp    - pointer to the structure for storing the CWE header fields
 *
 * Return:   TRUE if header is loaded successfully
 *           FALSE if bcSignature is not set correctly (only checked for APPL images)
 *
 * Notes:    Not all the fields in the header are read. Read more
 *           fields when necessary in the future
 *
 *           Except for the Signature field of an Application CWE header,
 *           this function does not validate the fields of the CWE header.
 *
 * Abort:    None
 *
 ************/
_global boolean cwe_header_load(
  uint8 * startp,
  struct cwe_header_s * hdp)
{
  uint8 *bufp;
  uint8 hdrver;
  enum cwe_image_type_e imagetype;

  if (hdp == NULL)
  {
    return FALSE;
  }

  /* init the buf pointer */
  bufp = startp;

  /* read in the required number of bytes from product specific buffer */
  pigetmulti(&bufp, hdp->prod_buf, sizeof(hdp->prod_buf));

  /* Get the Header Version: Set our pointer to the header revision number
   * first */
  bufp = startp + CWE_OFFSET_HDR_REV;

  /* Read the header version number */
  hdrver = piget32(&bufp);

  /* Continue reading the buffer from the Image Type Offset field */
  bufp = startp + CWE_OFFSET_IMG_TYPE;

  /* get the image type */
  hdp->image_type = piget32(&bufp);

  /* validate image type */
  (void)cwe_image_type_validate(hdp->image_type, &imagetype);

  /* get product type */
  hdp->prod_type = piget32(&bufp);

  /* get application image size */
  hdp->image_sz = piget32(&bufp);

  /* get CRC32 of application */
  hdp->image_crc = piget32(&bufp);

  /* get version string */
  pigetmulti(&bufp, hdp->version, CWE_VER_STR_SZ);

  /* get date string */
  pigetmulti(&bufp, hdp->rel_date, CWE_REL_DATE_SZ);

  /* get backwards compatibilty field */
  hdp->compat = piget32(&bufp);

  /* get the misc options if available */
  if (hdrver >= CWE_HEADER_VER)
  {
    hdp->misc_opts = *bufp;
  }
  else
  {
    hdp->misc_opts = 0;
  }

  /* get the load address and entry point based upon the header version.
   * Version 1 headers store these in network byte order while version 2 and
   * higher store them in little-endian */

  /* Network byte order */
  if (hdrver <= 1)
  {
    bufp = startp + CWE_OFFSET_STOR_ADDR;
    hdp->stor_addr = piget32(&bufp);

    bufp = startp + CWE_OFFSET_PROG_ADDR;
    hdp->prog_addr = piget32(&bufp);

    bufp = startp + CWE_OFFSET_ENTRY_PT;
    hdp->entry_pt = piget32(&bufp);
  }

  /* Little endian */
  else                          /* Versions 2 or higher */
  {
    bufp = startp + CWE_OFFSET_STOR_ADDR;
    hdp->stor_addr  = (uint32)(*bufp++);
    hdp->stor_addr |= (uint32)(*bufp++ << 8);
    hdp->stor_addr |= (uint32)(*bufp++ << 16);
    hdp->stor_addr |= (uint32)(*bufp++ << 24);

    bufp = startp + CWE_OFFSET_PROG_ADDR;
    hdp->prog_addr  = (uint32)(*bufp++);
    hdp->prog_addr |= (uint32)(*bufp++ << 8);
    hdp->prog_addr |= (uint32)(*bufp++ << 16);
    hdp->prog_addr |= (uint32)(*bufp++ << 24);

    bufp = startp + CWE_OFFSET_ENTRY_PT;
    hdp->entry_pt  = (uint32)(*bufp++);
    hdp->entry_pt |= (uint32)(*bufp++ << 8);
    hdp->entry_pt |= (uint32)(*bufp++ << 16);
    hdp->entry_pt |= (uint32)(*bufp++ << 24);
  }

  /* get signature */
  hdp->signature = piget32(&bufp);

  /* get product specific buffer CRC value */
  bufp = startp + CWE_OFFSET_PSB_CRC;
  hdp->psb_crc = piget32(&bufp);

  /* get CRC valid indicator value */
  bufp = startp + CWE_OFFSET_CRC_IND;
  hdp->crc_ind = piget32(&bufp);

  /* Only check the signature field for application imagetypes (not for
   * bootloader) since we always want to return FALSE for bootloader
   * imagetypes. */
  if (imagetype == CWE_IMAGE_TYPE_APPL)
  {
    /* check app signature */
    if (hdp->signature != CWE_SIGNATURE_APP)    
    {
      return (FALSE);
    }
  }

  /* default return value */
  return (TRUE);
}

/************
 *
 * Name:     cwe_image_validate
 *
 * Purpose:  Validate CWE image - image/product type, CRC, PSB CRC
 *
 * Parms:    (IN) hdp               - CWE header of the image
 *           (IN) imgp              - image pointer
 *           (IN) inputimgtype      - image type to check against
 *           (IN) validate_prodtype - if product type needs to be validated
 *           (IN) validate_crc      - if image CRC needs to be validated
 *
 * Return:   TRUE if CWE image and header are valid
 *           FALSE otherwise
 *
 * Abort:    None
 *
 * Notes:    
 *
 ************/
_global boolean cwe_image_validate(
  struct cwe_header_s * hdp,
  uint8               * data_p,
  enum cwe_image_type_e image_type,
  uint32                prod_type,
  boolean               validate_crc)
{
  enum cwe_image_type_e img_type_tmp;

  if ((hdp == NULL) || (data_p == NULL))
  {
    return FALSE;
  }

  if (!cwe_image_type_validate(hdp->image_type, &img_type_tmp))
  {
    dprintf(CRITICAL, "error cwe_image_type_validate\n");
    return FALSE;
  }

  /* Validate image type */
  if (CWE_IMAGE_TYPE_ANY != image_type && img_type_tmp != image_type)
  {
    dprintf(CRITICAL, "error cwe_image_type_validate!!!, type:%d, type_tmp:%d, prod:%x\n", 
                          image_type, img_type_tmp, hdp->prod_type);
    return FALSE;
  }

  /* Validate product type */
  if (prod_type && prod_type != hdp->prod_type)
  {
    dprintf(CRITICAL, "error prod_type\n");
    return FALSE;
  }

  /* validate PSB CRC */
  if (crcrc32(hdp->prod_buf, CWE_PROD_BUF_SZ, CRSTART_CRC32) != hdp->psb_crc)
  {
    dprintf(CRITICAL, "error crcrc32\n");
    return FALSE;
  }

  if (validate_crc && ((hdp->misc_opts & CWE_MISC_OPTS_COMPRESS) == 0))
  {
    /* validate image CRC */
    if (crcrc32(data_p, hdp->image_sz, CRSTART_CRC32) != hdp->image_crc)
    {
      dprintf(CRITICAL, "error crcrc32 2\n");
      return FALSE;
    }
  }

  return TRUE;
}


