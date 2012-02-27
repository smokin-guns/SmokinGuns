#include <assert.h>
#include "q3data.h"


/***************************************************
D!ABLO notes:

  the full range of ascii that Q3A will transmit in
  trap_SendServerCommand & trap_SendClientCommand is
  35 (#) to 127 ("+1)
  33 (!) also works.
  34 and below get stripped, while 128 and above are
  replaced with 46 (.)

  this code is designed to send ANY data across the
  network, adhereing to Q3's silly rules and
  reconstructing the code on the receiver.

  you can use this to send pure binary structures
  across the network and have them received intact
  on the other side.

***************************************************/


// low/middle/high plex flag characters
#define DTS_RLE_FLAG				33
#define DTS_LOW_FLAG				35
#define DTS_MID_FLAG				36
#define DTS_HIG_FLAG				37

// non-flagged usuable range of chars
#define DTS_LOWBYTE					38
#define DTS_HIGHBYTE				127

// offsets from real byte to get within range
#define DTS_LOW_OFS					-DTS_LOWBYTE
#define DTS_MID_OFS					DTS_HIGHBYTE
#define DTS_HIG_OFS					200


/***************************************************
PACKING / UNPACKING
***************************************************/

int DTS_PackData( byte *p_Data, int p_Len, byte *p_Pack )
{
	int cnt;
	int rle;

	// loop through and "pack" every byte in "data"
	for( cnt=0 ; DTS_MAX_PACKDATA>cnt && 0 < p_Len ; p_Len--, cnt++, p_Data++, p_Pack++ )
	{
			// run-length encoding
		for( rle=1 ; p_Data[rle] == p_Data[0] && DTS_MAX_CHANNELS > rle && 0 < p_Len ; rle++ )
			p_Len--;

		if( 1 < rle )
		{
			p_Len--;
			p_Pack[0] = DTS_RLE_FLAG;
			p_Pack[1] = ( rle + DTS_LOWBYTE );
			p_Pack += 2;
			p_Data += ( rle - 1 );
			cnt += 2;
		}

			// no packing needed
		if( DTS_LOWBYTE <= *p_Data && DTS_HIGHBYTE >= *p_Data )
		{
			*p_Pack = *p_Data;
			continue;
		}
			// in the low area
		if( DTS_LOWBYTE > *p_Data )
		{
			p_Pack[0] = DTS_LOW_FLAG;
			p_Pack[1] = (byte)( (int)(*p_Data) - DTS_LOW_OFS );
		}
		else // in the middle area
		if( DTS_HIG_OFS > *p_Data )
		{
			p_Pack[0] = DTS_MID_FLAG;
			p_Pack[1] = (byte)( (int)(*p_Data) - DTS_MID_OFS );
		}
		else // in the high area
		{
			p_Pack[0] = DTS_HIG_FLAG;
			p_Pack[1] = (byte)( (int)(*p_Data) - DTS_HIG_OFS );
		}

		cnt++; // copied 2 bytes in, increment again
		p_Pack++;
	}

	*p_Pack = 0; // terminate

	return cnt;
}

int DTS_UnpackData( byte* p_Data, byte* p_Unpack )
{
	int cnt;
	int rle;
	byte dec;
	byte* start = p_Unpack;

	// loop through and "unpack" every byte in "data"
	for( cnt=0 ; cnt<DTS_MAX_PACKDATA && *p_Data ; cnt++, p_Data++ )
	{
			// run-length decoding
		rle = 1;
		if( DTS_RLE_FLAG == *p_Data )
		{
			rle = ( p_Data[1] - DTS_LOWBYTE );
			p_Data += 2;
		}

		switch( *p_Data )
		{
		case DTS_LOW_FLAG: // in the low-area
			*p_Unpack = (byte)( (int)p_Data[1] + DTS_LOW_OFS );
			p_Data++; // it read 2 bytes out, increment again
			break;

		case DTS_MID_FLAG: // in the middle area
			*p_Unpack = (byte)( (int)p_Data[1] + DTS_MID_OFS );
			p_Data++; // it read 2 bytes out, increment again
			break;

		case DTS_HIG_FLAG: // in the high area
			*p_Unpack = (byte)( (int)p_Data[1] + DTS_HIG_OFS );
			p_Data++; // it read 2 bytes out, increment again
			break;

		default: // no unpacking needed
			*p_Unpack = *p_Data;
			break;
		}

			// run the length
		for( dec=*p_Unpack, p_Unpack++ ; 1<rle ; rle-- )
		{
			*p_Unpack = dec;
			p_Unpack++;
		}
	}

	return( p_Unpack - start );
}

//hit_header_t	*pheader = &header;
	//int		len;

	//byte	pData[DTS_MAX_PACKDATA+5];

	//memset( &pData, 0, DTS_MAX_PACKDATA );
	//len = DTS_PackData((byte*)pheader, sizeof(header), pData);
	//printf("%d %d %d\n", len, sizeof(header), sizeof (pData));

	//len = DTS_UnpackData(pData, (byte*)pheader);
	//printf("%d %d %d\n", len, sizeof(header), sizeof (pData));

	//printf("%d %d %d\n", pheader->numFrames, pheader->numSurfaces, pheader->numTags);

	//HEADER
	//SafeWrite(file, &g_data.model.numFrames, sizeof(int));
	//SafeWrite(file, &g_data.model.numSurfaces, sizeof(int));
	//SafeWrite(file, &g_data.model.numTags, sizeof(int));
