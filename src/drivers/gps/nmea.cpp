#include "nmea.h"
#ifdef WIN32
  #include <stdio.h>
  #include <io.h>
  #include <string.h>
  #include <time.h>
  uint64_t hrt_absolute_time()
  {
    return 0;
  }
  int GPS_Helper::set_baudrate(int const &,unsigned int){
    return 0;
  }
#else
  #include <unistd.h>
  #include <stdio.h>
  #include <poll.h>
  #include <math.h>
  #include <string.h>
  #include <assert.h>
  #include <systemlib/err.h>
  #include <uORB/uORB.h>
  #include <uORB/topics/vehicle_gps_position.h>
  #include <uORB/topics/satellite_info.h>
  #include <drivers/drv_hrt.h>
#endif

#include <fcntl.h>
#include <math.h>

typedef double float64_t;
typedef float  float32_t;

char*  str_scanDec(const char *pos, int8_t sign, int8_t n_max_digit, int32_t *result)
{
  int8_t n=0;
  int32_t d=0;
  int8_t neg = 0;

  if(*pos == '-') { neg = 1; pos++; }
  else if(*pos == '+')     { pos++; }
  else if(sign) return(NULL);
  while(*pos >= '0' && *pos <= '9')
  {
     d = d*10 + (*(pos++) - '0');
     n++;
     if(n_max_digit>0 && n==n_max_digit) break;
  }
  if(n==0 || n>10) return(NULL);
  if(neg) *result = -d;
  else    *result =  d;
  return((char *)pos);
}

char*  scanFloat64(const char *pos, int8_t sign, int8_t n_max_int, int8_t n_max_frac,
                       float64_t *result)
{
  float64_t f=0.0, div=1.0;
  int32_t d_int;
  int8_t n=0, isneg= 0;

  if( *pos == '-' ) isneg= 1;
  if((pos = str_scanDec(pos, sign, n_max_int, &d_int)) == NULL) return(NULL);
  if(*(pos) == '.')
  {
     pos++;
     while(*pos >= '0' && *pos <= '9')
     {
        f = f*(10.0) + (float32_t)(*(pos++) - '0');
        div *= (0.1);
        n++;
        if(n_max_frac>0 && n==n_max_frac) break;
     }
  }
  else if(n_max_frac > 0) return(NULL);
  if( isneg ) *result = (float64_t)d_int - f*div;
  else        *result = (float64_t)d_int + f*div;
  return((char *)pos);
}



NMEA::NMEA(const int &fd, struct vehicle_gps_position_s *gps_position, struct satellite_info_s *satellite_info):
_fd(fd),
_satellite_info(satellite_info),
_gps_position(gps_position)
{
    decode_init();  
    _decode_state = NME_DECODE_UNINIT;
    _rx_buffer_bytes = 0;
/*
    if (_fd_debug > 0) {
#ifndef WIN32
        warnx("Closing %d", _fd_debug);
#endif
        close(_fd_debug);
    }
    _fd_debug = open("/fs/microsd/gps_debug_nmea.bin", O_WRONLY | O_APPEND | O_CREAT );
    if(_fd_debug > 0)
      write(_fd_debug, "START\n",6);
#ifndef WIN32
    warnx("Got fd_debug %d", _fd_debug);
#endif
*/}

NMEA::~NMEA(){
/*
#ifndef WIN32
  warnx("Closing %d", _fd_debug);
#endif
  if(_fd_debug > 0)
    close(_fd_debug);
*/}
//All NMEA descriptions are taken from 
//http://www.trimble.com/OEM_ReceiverHelp/V4.44/en/NMEA-0183messages_MessageOverview.html
int NMEA::handle_message(int len){
  if(len < 7) return 0;
  int uiCalcComma = 0;
  for(int i = 0 ; i < len; i++){
    if(_rx_buffer[i] == ',')uiCalcComma++;
  }
  char* bufptr = (char*)( _rx_buffer+6);
  if((memcmp(_rx_buffer+3, "GGA,",3) == 0)&&(uiCalcComma == 14)){    
  /* 
    Time, position, and fix related data
    An example of the GBS message string is:

    $GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0,0031*4F

    Note - The data string exceeds the NMEA standard length.
    GGA message fields
    Field   Meaning
    0   Message ID $GPGGA
    1   UTC of position fix
    2   Latitude
    3   Direction of latitude: 
    N: North 
    S: South
    4   Longitude
    5   Direction of longitude: 
    E: East 
    W: West
    6   GPS Quality indicator: 
    0: Fix not valid 
    1: GPS fix 
    2: Differential GPS fix, OmniSTAR VBS 
    4: Real-Time Kinematic, fixed integers 
    5: Real-Time Kinematic, float integers, OmniSTAR XP/HP or Location RTK
    7   Number of SVs in use, range from 00 through to 24+
    8   HDOP
    9   Orthometric height (MSL reference)
    10  M: unit of measure for orthometric height is meters
    11  Geoid separation
    12  M: geoid separation measured in meters
    13  Age of differential GPS data record, Type 1 or Type 9. Null field when DGPS is not used.
    14  Reference station ID, range 0000-4095. A null field when any reference station ID is selected and no corrections are received1.
    15  
    The checksum data, always begins with *
    Note - If a user-defined geoid model, or an inclined
  */
      float64_t nmea_time = 0.0, lat = 0.0, lon = 0.0, alt = 0.0;
      int num_of_sv = 0, fix_quality=0, dgps_time = 0, dgps_baseid=0;
      float64_t hdop = 99.9;
      float64_t height_of_Geoid = 0.0;      
      char ns = '?', ew = '?';

      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&nmea_time);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&lat);
      if(bufptr && *(++bufptr) != ',') ns = *(bufptr++);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&lon);
      if(bufptr && *(++bufptr) != ',') ew = *(bufptr++);
      if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &fix_quality);
      if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &num_of_sv);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&hdop);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&alt);

      if(ns == 'S')
        lat = -lat;
      if(ew == 'W')
        lon = -lon;

      _gps_position->lat = (int(lat*0.01) + (lat*0.01 - int(lat*0.01))*100.0/60.0)*10000000;
      _gps_position->lon = (int(lon*0.01) + (lon*0.01 - int(lon*0.01))*100.0/60.0)*10000000;
      _gps_position->alt = alt*1000;
      _rate_count_lat_lon++;

      if((lat == 0.0) && (lon == 0.0) && (alt = 0.0)){
        _gps_position->fix_type = 0;
      }
      else
        _gps_position->fix_type = 3;
	    _gps_position->timestamp_position = hrt_absolute_time();

      int nmea_hour = nmea_time/10000;
      int nmea_minute = (nmea_time - nmea_hour*10000)/100;
      float64_t nmea_sec = nmea_time - nmea_hour*10000 - nmea_minute*100;
      //convert to unix timestamp
      struct tm timeinfo;
      timeinfo.tm_year = 2013 - 1900;
      timeinfo.tm_mon = 7 - 1; //7(July)
      timeinfo.tm_mday = 11;
      timeinfo.tm_hour = nmea_hour;
      timeinfo.tm_min = nmea_minute;
      timeinfo.tm_sec = int(nmea_sec);
      time_t epoch = mktime(&timeinfo);

      _gps_position->time_gps_usec = (uint64_t)epoch * 1000000; //TODO: test this
      _gps_position->time_gps_usec += (uint64_t)((nmea_sec - int(nmea_sec)) * 1e6);
      _gps_position->timestamp_time = hrt_absolute_time();

      _gps_position->eph = hdop;     
      _gps_position->epv = hdop*1.5; //Just to work (empirical);

      _gps_position->vel_m_s = 0;                                  /**< GPS ground speed (m/s) */
      _gps_position->vel_n_m_s = 0;                                /**< GPS ground speed in m/s */
      _gps_position->vel_e_m_s = 0;                                /**< GPS ground speed in m/s */
      _gps_position->vel_d_m_s = 0;                                /**< GPS ground speed in m/s */
      _gps_position->cog_rad = 0;                                  /**< Course over ground (NOT heading, but direction of movement) in rad, -PI..PI */
      _gps_position->vel_ned_valid = true;                             /**< Flag to indicate if NED speed is valid */
      _gps_position->c_variance_rad = 0.1;
      _gps_position->timestamp_velocity = hrt_absolute_time();
      return 1;
  }
  else if ((memcmp(_rx_buffer, "$PASHR,POS,", 11) == 0) && (uiCalcComma == 18)) {
  /*
Example $PASHR,POS,2,10,125410.00,5525.8138702,N,03833.9587380,E,131.555,1.0,0.0,0.007,-0.001,2.0,1.0,1.7,1.0,*34

      $PASHR,POS,d1,d2,m3,m4,c5,m6,c7,f8,f9,f10,f11,f12,f13,f14,f15,f16,s17*cc 
      Parameter Description Range 
        d1 Position mode 0: standalone 
                         1: differential 
                         2: RTK float 
                         3: RTK fixed 
                         5: Dead reckoning 
                         9: SBAS (see NPT setting) 
        d2 Number of satellite used in position fix 0-99
        m3 Current UTC time of position fix (hhmmss.ss) 000000.00-235959.99 
        m4 Latitude of position (ddmm.mmmmmm) 0-90 degrees 00-59.9999999 minutes 
        c5 Latitude sector N, S
        m6 Longitude of position (dddmm.mmmmmm) 0-180 degrees 00-59.9999999 minutes 
        c7 Longitude sector E,W
        f8 Altitude above ellipsoid +9999.000
        f9 Differential age (data link age), seconds 0.0-600.0
        f10 True track/course over ground in degrees 0.0-359.9
        f11 Speed over ground in knots 0.0-999.9
        f12 Vertical velocity in decimeters per second +999.9 
        f13 PDOP 0-99.9 
        f14 HDOP 0-99.9 
        f15 VDOP 0-99.9 
        f16 TDOP 0-99.9 
        s17 Reserved no data 
        *cc Checksum 
      */
      bufptr = (char*)( _rx_buffer+10);
      float64_t nmea_time = 0.0, lat = 0.0, lon = 0.0, alt = 0.0;
      int num_of_sv = 0, fix_quality=0, dgps_baseid=0;
      float64_t track_true = 0.0, ground_speed = 0.0 , age_of_corr = 0.0;
      float64_t hdop = 99.9, vdop = 99.9,  pdop = 99.9, tdop = 99.9,vertic_vel = 0.0;
      float64_t height_of_Geoid = 0.0;      
      char ns = '?', ew = '?';

      if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &fix_quality);
      if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &num_of_sv);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&nmea_time);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&lat);
      if(bufptr && *(++bufptr) != ',') ns = *(bufptr++);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&lon);
      if(bufptr && *(++bufptr) != ',') ew = *(bufptr++);      
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&alt);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&age_of_corr);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&track_true);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&ground_speed);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&vertic_vel);      
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&pdop);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&hdop);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&vdop);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&tdop);

      if(ns == 'S')
        lat = -lat;
      if(ew == 'W')
        lon = -lon;

      _gps_position->lat = (int(lat*0.01) + (lat*0.01 - int(lat*0.01))*100.0/60.0)*10000000;
      _gps_position->lon = (int(lon*0.01) + (lon*0.01 - int(lon*0.01))*100.0/60.0)*10000000;
      _gps_position->alt = alt*1000;
      _rate_count_lat_lon++;

      if((lat == 0.0) && (lon == 0.0) && (alt = 0.0)){
        _gps_position->fix_type = 0;
      }
      else
        _gps_position->fix_type = 3;
	    _gps_position->timestamp_position = hrt_absolute_time();

      _gps_position->fix_quality = fix_quality;

      int nmea_hour = nmea_time/10000;
      int nmea_minute = (nmea_time - nmea_hour*10000)/100;
      float64_t nmea_sec = nmea_time - nmea_hour*10000 - nmea_minute*100;
      //convert to unix timestamp
      struct tm timeinfo;
      timeinfo.tm_year = 2013 - 1900;
      timeinfo.tm_mon = 7 - 1; //7(July)
      timeinfo.tm_mday = 11;
      timeinfo.tm_hour = nmea_hour;
      timeinfo.tm_min = nmea_minute;
      timeinfo.tm_sec = int(nmea_sec);
      time_t epoch = mktime(&timeinfo);

      _gps_position->time_gps_usec = (uint64_t)epoch * 1000000; //TODO: test this
      _gps_position->time_gps_usec += (uint64_t)((nmea_sec - int(nmea_sec)) * 1e6);
      _gps_position->timestamp_time = hrt_absolute_time();

      _gps_position->eph = hdop;     
      _gps_position->epv = vdop;

   
      const float64_t m_pi  = 3.14159265;

      double track_rad = track_true * m_pi / 180.0;

	  	double velocity_ms = ground_speed  / 3.6;
		  double velocity_north = velocity_ms * cos(track_rad);
		  double velocity_east  = velocity_ms * sin(track_rad);

		  _gps_position->vel_m_s = velocity_ms;                                  /**< GPS ground speed (m/s) */
		  _gps_position->vel_n_m_s = velocity_north;                                /**< GPS ground speed in m/s */
		  _gps_position->vel_e_m_s = velocity_east;                                /**< GPS ground speed in m/s */
		  _gps_position->vel_d_m_s = -vertic_vel*0.1;                                /**< GPS ground speed in m/s */
		  _gps_position->cog_rad = track_rad;                                  /**< Course over ground (NOT heading, but direction of movement) in rad, -PI..PI */
		  _gps_position->vel_ned_valid = true;                             /**< Flag to indicate if NED speed is valid */
		  _gps_position->c_variance_rad = 0.1;
		  _gps_position->timestamp_velocity = hrt_absolute_time();
      return 1;

  }
  else if((memcmp(_rx_buffer+3, "GST,",3) == 0)&&(uiCalcComma == 8)){
  /* 
    Position error statistics
    An example of the GST message string is:

    $GPGST,172814.0,0.006,0.023,0.020,273.6,0.023,0.020,0.031*6A

    The Talker ID ($--) will vary depending on the satellite system used for the position solution:

    $GP - GPS only
    $GL - GLONASS only
    $GN - Combined
    GST message fields
    Field   Meaning
    0   Message ID $GPGST
    1   UTC of position fix
    2   RMS value of the pseudorange residuals; includes carrier phase residuals during periods of RTK (float) and RTK (fixed) processing
    3   Error ellipse semi-major axis 1 sigma error, in meters
    4   Error ellipse semi-minor axis 1 sigma error, in meters
    5   Error ellipse orientation, degrees from true north
    6   Latitude 1 sigma error, in meters
    7   Longitude 1 sigma error, in meters
    8   Height 1 sigma error, in meters
    9   The checksum data, always begins with *
  */
      float64_t nmea_time = 0.0, lat_err = 0.0, lon_err = 0.0, alt_err = 0.0;
      float64_t min_err = 0.0, maj_err = 0.0, deg_from_north = 0.0, rms_err = 0.0;

      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&nmea_time);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&rms_err);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&maj_err);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&min_err);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&deg_from_north);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&lat_err);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&lon_err);
      if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&alt_err);

      _gps_position->s_variance_m_s = 0;
      _gps_position->timestamp_variance = hrt_absolute_time();
  }
  else if((memcmp(_rx_buffer+3, "VTG,",3) == 0)&&(uiCalcComma == 9)){
  /*
    Track made good and speed over ground
    An example of the VTG message string is:

    $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48

    VTG message fields
    Field   Meaning
    0   Message ID $GPVTG
    1   Track made good (degrees true)
    2   T: track made good is relative to true north
    3   Track made good (degrees magnetic)
    4   M: track made good is relative to magnetic north
    5   Speed, in knots
    6   N: speed is measured in knots
    7   Speed over ground in kilometers/hour (kph)
    8   K: speed over ground is measured in kph
    9   The checksum data, always begins with *
  */
     float64_t track_true = 0.0, track_magnetic = 0.0, speed = 0.0, ground_speed = 0.0;
     char true_north = '?', magnetic_north = '?', speed_in_knots = '?', speed_in_kph  = '?';
     
     if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&track_true);
     if(bufptr && *(++bufptr) != ',') true_north = *(bufptr++);
     if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&track_magnetic);
     if(bufptr && *(++bufptr) != ',') magnetic_north = *(bufptr++);
     if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&speed);
     if(bufptr && *(++bufptr) != ',') speed_in_knots = *(bufptr++);
     if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&ground_speed);
     if(bufptr && *(++bufptr) != ',') speed_in_kph = *(bufptr++);

     const float64_t dPI  = 3.14159265;
		 float64_t tan_C = tan(track_true * dPI / 180.0);              
		 float64_t lat_ = sqrt(ground_speed*ground_speed/ (1+tan_C)); //km/hour
		 float64_t lon_ = lat_*tan_C; //		 	                        //km/hour
  }
  else if((memcmp(_rx_buffer+3, "GSV,",3) == 0)){
  /*
    The GSV message string identifies the number of SVs in view, the PRN numbers, elevations, azimuths, and SNR values. An example of the GSV message string is:

    $GPGSV,4,1,13,02,02,213,,03,-3,000,,11,00,121,,14,13,172,05*67

    GSV message fields
    Field   Meaning
    0   Message ID $GPGSV
    1   Total number of messages of this type in this cycle
    2   Message number
    3   Total number of SVs visible
    4   SV PRN number
    5   Elevation, in degrees, 90� maximum
    6   Azimuth, degrees from True North, 000� through 359�
    7   SNR, 00 through 99 dB (null when not tracking)
    8-11    Information about second SV, same format as fields 4 through 7
    12-15   Information about third SV, same format as fields 4 through 7
    16-19   Information about fourth SV, same format as fields 4 through 7
    20  The checksum data, always begins with *
  */
// currently process only gps, because do not know what 
// Global satellite ID I should use for non GPS sats
    bool bGPS = false;
    if(memcmp(_rx_buffer, "$GP",3) != 0) 
      return 0;
    else
      bGPS = true;

    int all_msg_num, this_msg_num, tot_sv_visible;
    struct gsv_sat{
      int svid;
      int elevation;
      int azimuth;
      int snr;
    } sat[4];
    memset(sat, 0, sizeof(sat));
    
    if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &all_msg_num);
    if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &this_msg_num);
    if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &tot_sv_visible);
    if((this_msg_num<1) || (this_msg_num>all_msg_num)){
      return 0;
    }
    if((this_msg_num == 0)&&(bGPS == true)){
      memset(_satellite_info->svid,      0,sizeof(_satellite_info->svid));
      memset(_satellite_info->used,     0,sizeof(_satellite_info->used));
      memset(_satellite_info->snr,      0,sizeof(_satellite_info->snr));
      memset(_satellite_info->elevation,0,sizeof(_satellite_info->elevation));
      memset(_satellite_info->azimuth,  0,sizeof(_satellite_info->azimuth));
    }

    int end = 4;
    if(this_msg_num == all_msg_num){
      end =  tot_sv_visible - (this_msg_num-1)*4;
      //_gps_position->satellite_info_available = 1;
      _gps_position->satellites_used = this_msg_num;
      _satellite_info->count = tot_sv_visible;
      _satellite_info->timestamp = hrt_absolute_time();
    }
    for(int y = 0 ; y < end ;y++){
      if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &sat[y].svid);
      if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &sat[y].elevation);
      if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &sat[y].azimuth);
      if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &sat[y].snr);
    
      _satellite_info->svid[y+(this_msg_num-1)*4]       = sat[y].svid;
      _satellite_info->used[y+(this_msg_num-1)*4]      = ((sat[y].snr>0)? true: false);
      _satellite_info->snr[y+(this_msg_num-1)*4]       = sat[y].snr;
      _satellite_info->elevation[y+(this_msg_num-1)*4] = sat[y].elevation;
      _satellite_info->azimuth[y+(this_msg_num-1)*4]   = sat[y].azimuth;
    }
  }
  else if((memcmp(_rx_buffer+3, "ZDA,",3) == 0)&&(uiCalcComma == 6)){
    /*
    UTC day, month, and year, and local time zone offset
    An example of the ZDA message string is:

    $GPZDA,172809.456,12,07,1996,00,00*45

    ZDA message fields
    Field	Meaning
    0	Message ID $GPZDA
    1	UTC
    2	Day, ranging between 01 and 31
    3	Month, ranging between 01 and 12
    4	Year
    5	Local time zone offset from GMT, ranging from 00 through �13 hours
    6	Local time zone offset from GMT, ranging from 00 through 59 minutes
    7	The checksum data, always begins with *
    Fields 5 and 6 together yield the total offset. For example, if field 5 is -5 and field 6 is +15, local time is 5 hours and 15 minutes earlier than GMT.     
    */
    float64_t nmea_time = 0.0;
    int day = 0, month = 0,year = 0,local_time_off_hour = 0,local_time_off_min = 0; 
    if(bufptr && *(++bufptr) != ',') bufptr = scanFloat64(bufptr, 0, 9, 9,&nmea_time);
    if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &day);
    if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &month);
    if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &year);
    if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &local_time_off_hour);
    if(bufptr && *(++bufptr) != ',') bufptr = str_scanDec(bufptr, 0, 9, &local_time_off_min);


    int nmea_hour = nmea_time/10000;
    int nmea_minute = (nmea_time - nmea_hour*10000)/100;
    float64_t nmea_sec = nmea_time - nmea_hour*10000 - nmea_minute*100;
    //convert to unix timestamp
    struct tm timeinfo;
    timeinfo.tm_year = 2013 - 1900;
    timeinfo.tm_mon = 7 - 1; //7(July)
    timeinfo.tm_mday = 11;
    timeinfo.tm_hour = nmea_hour;
    timeinfo.tm_min = nmea_minute;
    timeinfo.tm_sec = int(nmea_sec);
    time_t epoch = mktime(&timeinfo);

    _gps_position->time_gps_usec = (uint64_t)epoch * 1000000; //TODO: test this
    _gps_position->time_gps_usec += (uint64_t)((nmea_sec - int(nmea_sec)) * 1e6);
    _gps_position->timestamp_time = hrt_absolute_time();
  }

  return 0;
}


int NMEA::receive(unsigned timeout){
{
    /* poll descriptor */
    pollfd fds[1];
    fds[0].fd = _fd;
    fds[0].events = POLLIN;

    uint8_t buf[32];

    /* timeout additional to poll */
    uint64_t time_started = hrt_absolute_time();

    int j = 0;
    ssize_t count = 0;

    while (true) {

        /* pass received bytes to the packet decoder */
        while (j < count) {
            int l=0;
            if ((l = parse_char(buf[j])) > 0) {
                /* return to configure during configuration or to the gps driver during normal work
                 * if a packet has arrived */
                 if (handle_message(l) > 0)
                    return 1;
            }
            /* in case we keep trying but only get crap from GPS */
            if (time_started + timeout*1000*2 < hrt_absolute_time() ) {
                return -1;
            }
            j++;
        }

        /* everything is read */
        j = count = 0;

        /* then poll for new data */
        int ret = ::poll(fds, sizeof(fds) / sizeof(fds[0]), timeout*2);

        if (ret < 0) {
            /* something went wrong when polling */
            return -1;

        } else if (ret == 0) {
            /* Timeout */
            return -1;

        } else if (ret > 0) {
            /* if we have new data from GPS, go handle it */
            if (fds[0].revents & POLLIN) {
                /*
                 * We are here because poll says there is some data, so this
                 * won't block even on a blocking device.  If more bytes are
                 * available, we'll go back to poll() again...
                 */
                count = ::read(_fd, buf, sizeof(buf));
//                if(count){
//                  write(_fd_debug, buf, count);
//                  fsync(_fd_debug);
//                }

            }
        }
    }
}

}
#define HEXDIGIT_CHAR(d) ((char)((d) + (((d) < 0xA) ? '0' : 'A'-0xA)))

int NMEA::parse_char(uint8_t b)
{
	int iRet = 0;

	switch (_decode_state) {
		/* First, look for sync1 */
	case NME_DECODE_UNINIT:
		if (b == '$') {
			_decode_state = NME_DECODE_GOT_SYNC1;
			_rx_buffer_bytes = 0;
			_rx_buffer[_rx_buffer_bytes++] = b;
		}

		break;

	case NME_DECODE_GOT_SYNC1:
		if (b == '$') {
			_decode_state = NME_DECODE_GOT_SYNC1;
			_rx_buffer_bytes = 0;

		} else if (b == '*') {
			_decode_state = NME_DECODE_GOT_ASTERIKS;
		}

		if (_rx_buffer_bytes >= (sizeof(_rx_buffer) - 5)) {
			_decode_state = NME_DECODE_UNINIT;
			_rx_buffer_bytes = 0;

		} else
			_rx_buffer[_rx_buffer_bytes++] = b;

		break;

	case NME_DECODE_GOT_ASTERIKS:
		_rx_buffer[_rx_buffer_bytes++] = b;
		_decode_state = NME_DECODE_GOT_FIRST_CS_BYTE;
		break;

	case NME_DECODE_GOT_FIRST_CS_BYTE:
		_rx_buffer[_rx_buffer_bytes++] = b;
		uint8_t checksum = 0;
		uint8_t *buffer = _rx_buffer + 1;
		uint8_t *bufend = _rx_buffer + _rx_buffer_bytes - 3;

		for (; buffer < bufend; buffer++) checksum ^= *buffer;

		if ((HEXDIGIT_CHAR(checksum >> 4) == *(_rx_buffer + _rx_buffer_bytes - 2)) &&
		    (HEXDIGIT_CHAR(checksum & 0x0F) == *(_rx_buffer + _rx_buffer_bytes - 1))) {
			iRet = _rx_buffer_bytes;
		}

		_decode_state = NME_DECODE_UNINIT;
		_rx_buffer_bytes = 0;
		break;
	}

	return iRet;
}

void NMEA::decode_init(void){

}

//ashtech boad configuration script 
//char comm[] = "$PASHS,NME,GGA,A,ON,0.1\r\n"
char comm[] = "$PASHS,NME,GGA,B,OFF\r\n"\
              "$PASHS,NME,GST,B,ON,3\r\n"\
              "$PASHS,NME,POS,B,ON,0.1\r\n"\
              "$PASHS,NME,GSV,B,ON,3\r\n"\
              "$PASHS,NME,ZDA,B,ON,3\r\n"\
              "$PASHS,SPD,A,7\r\n"\
              "$PASHS,SPD,B,7\r\n"; // default baud is 7

int NMEA::configure(unsigned &baudrate){
    /* try different baudrates */
    const unsigned baudrates_to_try[] = {9600, 38400, 19200, 57600, 115200};

    for (int baud_i = 0; baud_i < sizeof(baudrates_to_try)/sizeof(baudrates_to_try[0]); baud_i++) {
      baudrate = baudrates_to_try[baud_i];
      set_baudrate(_fd, baudrate);
      write(_fd, (uint8_t*)comm, sizeof(comm));
    }
    set_baudrate(_fd, 38400);
    return 0;
}
