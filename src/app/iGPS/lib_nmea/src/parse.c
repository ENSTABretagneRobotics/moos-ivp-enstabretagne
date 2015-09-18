/*
 * This file is part of nmealib.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nmea/parse.h>

#include <nmea/gmath.h>
#include <nmea/tok.h>
#include <nmea/context.h>

#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>

/** the size of the buffer to put time string (that is to be parsed) into */
#define NMEA_TIMEPARSE_BUF  256

/**
 * Parse nmeaTIME (time only, no date) from a string.
 * The format that is used (hhmmss, hhmmss.s, hhmmss.ss or hhmmss.sss) is
 * determined by the length of the string.
 *
 * @param s the string
 * @param len the length of the string
 * @param t a pointer to the nmeaTIME structure in which to store the parsed time
 * @return true on success, false otherwise
 */
static bool _nmea_parse_time(const char *s, const int len, nmeaTIME *t) {
  assert(s);
  assert(t);

  if (len == (sizeof("hhmmss") - 1)) {
    t->hsec = 0;
    return (3 == nmea_scanf(s, len, "%2d%2d%2d", &t->hour, &t->min, &t->sec));
  }

  if (len == (sizeof("hhmmss.s") - 1)) {
    if (4 == nmea_scanf(s, len, "%2d%2d%2d.%d", &t->hour, &t->min, &t->sec, &t->hsec)) {
      t->hsec *= 10;
      return true;
    }
    return false;
  }

  if (len == (sizeof("hhmmss.ss") - 1)) {
    return (4 == nmea_scanf(s, len, "%2d%2d%2d.%d", &t->hour, &t->min, &t->sec, &t->hsec));
  }

  if (len == (sizeof("hhmmss.sss") - 1)) {
    if ((4 == nmea_scanf(s, len, "%2d%2d%2d.%d", &t->hour, &t->min, &t->sec, &t->hsec))) {
      t->hsec = (t->hsec + 9) / 10;
      return true;
    }
    return false;
  }

  nmea_error("Parse error: invalid time format in %s", s);
  return false;
}

/**
 * Parse nmeaTIME (date only, no time) from a string.
 * The month is adjusted -1 to comply with the nmeaTIME month range of [0, 11].
 * The year is adjusted +100 for years before 90 to comply with the nmeaTIME
 * year range of [90, 189].
 *
 * @param date the date
 * @param t a pointer to the nmeaTIME structure in which to store the parsed date
 * @return true on success, false otherwise
 */
static bool _nmea_parse_date(const int date, nmeaTIME *t) {
  assert(t);

  if ((date < 0) || (date > 999999)) {
    nmea_error("Parse error: invalid time format in %d", date);
    return false;
  }

  t->day = date / 10000;
  t->mon = (date / 100) % 100;
  t->mon--;
  t->year = date % 100;
  if (t->year < 90) {
    t->year += 100;
  }

  return true;
}

/**
 * Validate the time fields in an nmeaTIME structure.
 * Expects:
 * <pre>
 *   0 <= hour <   24
 *   0 <= min  <   60
 *   0 <= sec  <=  60
 *   0 <= hsec <  100
 * </pre>
 *
 * @param t a pointer to the structure
 * @return true when valid, false otherwise
 */
static bool validateTime(const nmeaTIME * t) {
  if (!t) {
    return false;
  }

  if (!((t->hour >= 0) && (t->hour < 24) && (t->min >= 0) && (t->min < 60) && (t->sec >= 0) && (t->sec <= 60)
      && (t->hsec >= 0) && (t->hsec < 100))) {
    nmea_error("Parse error: invalid time (%d:%d:%d.%d)", t->hour, t->min, t->sec, t->hsec);
    return false;
  }

  return true;
}

/**
 * Validate the date fields in an nmeaTIME structure.
 * Expects:
 * <pre>
 *   year  [90, 189]
 *   month [ 0,  11]
 *   day   [ 1,  31]
 * </pre>
 *
 * @param t a pointer to the structure
 * @return true when valid, false otherwise
 */
static bool validateDate(const nmeaTIME * t) {
  if (!t) {
    return false;
  }

  if (!((t->year >= 90) && (t->year <= 189) && (t->mon >= 0) && (t->mon <= 11) && (t->day >= 1) && (t->day <= 31))) {
    nmea_error("Parse error: invalid date (%d-%d-%d - D-M-Y)", t->day, t->mon, t->year);
    return false;
  }

  return true;
}

/**
 * Validate north/south or east/west and uppercase it.
 * Expects:
 * <pre>
 *   c in { n, N, s, S } (for north/south)
 *   c in { e, E, w, W } (for east/west)
 * </pre>
 *
 * @param c a pointer to the character. The character will be converted to uppercase.
 * @param ns true: evaluate north/south, false: evaluate east/west
 * @return true when valid, false otherwise
 */
static bool validateNSEW(char * c, const bool ns) {
  if (!c) {
    return false;
  }

  *c = toupper(*c);

  if (ns) {
    if (!((*c == 'N') || (*c == 'S'))) {
      nmea_error("Parse error: invalid north/south (%c)", *c);
      return false;
    }
  } else {
    if (!((*c == 'E') || (*c == 'W'))) {
      nmea_error("Parse error: invalid east/west (%c)", *c);
      return false;
    }
  }

  return true;
}

/**
 * Uppercase mode and validate it.
 * Expects:
 * <pre>
 *   c in { A, D, E, F, M, N, P, R, S }
 *
 *   A = Autonomous. Satellite system used in non-differential mode in position fix
 *   D = Differential. Satellite system used in differential mode in position fix
 *   E = Estimated (dead reckoning) mode
 *   F = Float RTK. Satellite system used in real time kinematic mode with floating integers
 *   M = Manual input mode
 *   N = No fix. Satellite system not used in position fix, or fix not valid
 *   P = Precise. Satellite system used in precision mode. Precision mode is defined
 *       as no deliberate degradation (such as Selective Availability) and higher
 *       resolution code (P-code) is used to compute position fix.
 *   R = Real Time Kinematic. Satellite system used in RTK mode with fixed integers
 *   S = Simulator mode
 * </pre>
 *
 * @param c a pointer to the character. The character will be converted to uppercase.
 * @return true when valid, false otherwise
 */
static bool validateMode(char * c) {
  if (!c) {
    return false;
  }

  *c = toupper(*c);

  if (!((*c == 'A') || (*c == 'D') || (*c == 'E') || (*c == 'F') || (*c == 'M') || (*c == 'N') || (*c == 'P')
      || (*c == 'R') || (*c == 'S'))) {
    nmea_error("Parse error: invalid mode (%c)", *c);
    return false;
  }

  return true;
}

/**
 * Determine whether the given character is not allowed in an NMEA string.
 *
 * @param c
 * The character to check
 *
 * @return
 * - a pointer to the invalid character name/description when the string has invalid characters
 * - NULL otherwise
 */
const char * isInvalidNMEACharacter(const char * c) {
  static const char * invalidNonAsciiCharsName = "non-ascii character";
  static const char invalidChars[] = {
      '$',
      '*',
      '!',
      '\\',
      '^',
      '~'
  };
  static const char * invalidCharsNames[] = {
      "sentence delimiter ($)",
      "checksum field delimiter (*)",
      "exclamation mark (!)",
      "backslash (\\)",
      "power (^)",
      "tilde (~)"
  };

  size_t charIndex;

  if (!((*c >= 32) && (*c <= 126))) {
    return invalidNonAsciiCharsName;
  }

  for (charIndex = 0; charIndex < sizeof(invalidChars); charIndex++) {
    if (*c == invalidChars[charIndex]) {
      return invalidCharsNames[charIndex];
    }
  }

  return NULL;
}

/**
 * Determine whether the given string contains characters that are not allowed
 * in an NMEA string.
 *
 * @param s
 * The string to check
 * @param len
 * The length of the string to check
 *
 * @return
 * - a pointer to the invalid character name/description when the string has invalid characters
 * - NULL otherwise
 */
const char * nmea_parse_sentence_has_invalid_chars(const char * s, const size_t len) {
  size_t i;

  if (!s || !len) {
    return NULL;
  }

  for (i = 0; i < len; i++) {
    const char * invalidCharName = isInvalidNMEACharacter(&s[i]);
    if (invalidCharName) {
      return invalidCharName;
    }
  }

  return NULL;
}

/**
 * Determine sentence type (see nmeaPACKTYPE) by the header of a string.
 * The header is the start of an NMEA sentence, right after the $.
 *
 * @param s the string. must be the NMEA string, right after the initial $
 * @param len the length of the string
 * @return The packet type (or GPNON when it could not be determined)
 */
enum nmeaPACKTYPE nmea_parse_get_sentence_type(const char *s, const int len) {
  //static const char *pheads[] = { "GPGGA", "GPGSA", "GPGSV", "GPRMC", "GPVTG" };
  //static const char *pheads[] = { "GPGGA", "GPGSA", "GPGSV", "GPRMC", "GPVTG", "GNGGA", "GNGSA", "GNGSV", "GNRMC", "GNVTG" };
  static const char *pheads[] = { "GGA", "GSA", "GSV", "RMC", "VTG", "PRTI01"};
  static const int pheads_offset[] = {2,2,2,2,2,0};
  static const enum nmeaPACKTYPE types[] = { GPGGA, GPGSA, GPGSV, GPRMC, GPVTG, PRTI01};
  unsigned int i;

  assert(s);

  if (len < 5) {
    return GPNON;
  }

  for (i = 0; i < sizeof(pheads)/sizeof(pheads[0]); i++) {
    if (memcmp(&s[pheads_offset[i]], pheads[i], strlen(pheads[i]))==0) {
      return types[i];
    }
  }

  return GPNON;
}

/**
 * Parse a GPGGA sentence from a string
 *
 * @param s the string
 * @param len the length of the string
 * @param has_checksum true when the string contains a checksum
 * @param pack a pointer to the result structure
 * @return 1 (true) - if parsed successfully or 0 (false) otherwise.
 */
int nmea_parse_GPGGA(const char *s, const int len, bool has_checksum, nmeaGPGGA *pack) {
  int token_count;
  char time_buff[NMEA_TIMEPARSE_BUF];
  size_t time_buff_len = 0;

  if (!has_checksum) {
    return 0;
  }

  assert(s);
  assert(pack);

  nmea_trace_buff(s, len);

  /*
   * Clear before parsing, to be able to detect absent fields
   */
  time_buff[0] = '\0';
  pack->present = 0;
  pack->utc.hour = -1;
  pack->utc.min = -1;
  pack->utc.sec = -1;
  pack->utc.hsec = -1;
  pack->lat = NAN;
  pack->ns = 0;
  pack->lon = NAN;
  pack->ew = 0;
  pack->sig = -1;
  pack->satinuse = -1;
  pack->HDOP = NAN;
  pack->elv = NAN;
  pack->elv_units = 0;
  pack->diff = 0;     /* ignored */
  pack->diff_units = 0; /* ignored */
  pack->dgps_age = 0;   /* ignored */
  pack->dgps_sid = 0;   /* ignored */

  /* parse */
  token_count = nmea_scanf(s, len, "$%*%*GGA,%s,%f,%c,%f,%c,%d,%d,%f,%f,%c,%f,%c,%f,%d*", &time_buff[0], &pack->lat,
      &pack->ns, &pack->lon, &pack->ew, &pack->sig, &pack->satinuse, &pack->HDOP, &pack->elv, &pack->elv_units,
      &pack->diff, &pack->diff_units, &pack->dgps_age, &pack->dgps_sid);

  /* see that we have enough tokens */
  if (token_count != 14) {
    nmea_error("GPGGA parse error: need 14 tokens, got %d in %s", token_count, s);
    return 0;
  }

  /* determine which fields are present and validate them */

  time_buff_len = strlen(&time_buff[0]);
  if (time_buff_len > (NMEA_TIMEPARSE_BUF - 1))
    time_buff_len = NMEA_TIMEPARSE_BUF - 1;
  if (time_buff_len) {
    if (!_nmea_parse_time(&time_buff[0], time_buff_len, &pack->utc)) {
      return 0;
    }

    if (!validateTime(&pack->utc)) {
      return 0;
    }

    nmea_INFO_set_present(&pack->present, UTCTIME);
  }
  if (!isnan(pack->lat) && (pack->ns)) {
    if (!validateNSEW(&pack->ns, true)) {
      return 0;
    }

    nmea_INFO_set_present(&pack->present, LAT);
  }
  if (!isnan(pack->lon) && (pack->ew)) {
    if (!validateNSEW(&pack->ew, false)) {
      return 0;
    }

    nmea_INFO_set_present(&pack->present, LON);
  }
  if (pack->sig != -1) {
    if (!((pack->sig >= NMEA_SIG_FIRST) && (pack->sig <= NMEA_SIG_LAST))) {
      nmea_error("GPGGA parse error: invalid signal %d, expected [%d, %d]", pack->sig, NMEA_SIG_FIRST, NMEA_SIG_LAST);
      return 0;
    }

    nmea_INFO_set_present(&pack->present, SIG);
  }
  if (pack->satinuse != -1) {
    nmea_INFO_set_present(&pack->present, SATINUSECOUNT);
  }
  if (!isnan(pack->HDOP)) {
    nmea_INFO_set_present(&pack->present, HDOP);
  }
  if (!isnan(pack->elv) && (pack->elv_units)) {
    if (pack->elv_units != 'M') {
      nmea_error("GPGGA parse error: invalid elevation unit (%c)", pack->elv_units);
      return 0;
    }

    nmea_INFO_set_present(&pack->present, ELV);
  }
  /* ignore diff and diff_units */
  /* ignore dgps_age and dgps_sid */

  return 1;
}

/**
 * Parse a GPGSA sentence from a string
 *
 * @param s the string
 * @param len the length of the string
 * @param has_checksum true when the string contains a checksum
 * @param pack a pointer to the result structure
 * @return 1 (true) - if parsed successfully or 0 (false) otherwise.
 */
int nmea_parse_GPGSA(const char *s, const int len, bool has_checksum, nmeaGPGSA *pack) {
  int token_count;
  int i;

  if (!has_checksum) {
    return 0;
  }

  assert(s);
  assert(pack);

  nmea_trace_buff(s, len);

  /*
   * Clear before parsing, to be able to detect absent fields
   */
  pack->present = 0;
  pack->fix_mode = 0;
  pack->fix_type = -1;
  for (i = 0; i < NMEA_MAXSAT; i++) {
    pack->sat_prn[i] = 0;
  }
  pack->PDOP = NAN;
  pack->HDOP = NAN;
  pack->VDOP = NAN;

  /* parse */
  token_count = nmea_scanf(s, len, "$%*%*GSA,%c,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f*", &pack->fix_mode,
      &pack->fix_type, &pack->sat_prn[0], &pack->sat_prn[1], &pack->sat_prn[2], &pack->sat_prn[3],
      &pack->sat_prn[4], &pack->sat_prn[5], &pack->sat_prn[6], &pack->sat_prn[7], &pack->sat_prn[8],
      &pack->sat_prn[9], &pack->sat_prn[10], &pack->sat_prn[11], &pack->PDOP, &pack->HDOP, &pack->VDOP);

  /* see that we have enough tokens */
  if (token_count != 17) {
    nmea_error("GPGSA parse error: need 17 tokens, got %d in %s", token_count, s);
    return 0;
  }

  /* determine which fields are present and validate them */
  pack->fix_mode = toupper(pack->fix_mode);
  if (!((pack->fix_mode == 'A') || (pack->fix_mode == 'M'))) {
    nmea_error("GPGSA parse error: invalid fix mode (%c)", pack->fix_mode);
    return 0;
  }
  if (pack->fix_type != -1) {
    if (!((pack->fix_type >= NMEA_FIX_FIRST) && (pack->fix_type <= NMEA_FIX_LAST))) {
      nmea_error("GPGSA parse error: invalid fix type %d, expected [%d, %d]", pack->fix_type, NMEA_FIX_FIRST, NMEA_FIX_LAST);
      return 0;
    }

    nmea_INFO_set_present(&pack->present, FIX);
  }
  for (i = 0; i < NMEA_MAXSAT; i++) {
    if (pack->sat_prn[i] != 0) {
      nmea_INFO_set_present(&pack->present, SATINUSE);
      break;
    }
  }
  if (!isnan(pack->PDOP)) {
    nmea_INFO_set_present(&pack->present, PDOP);
  }
  if (!isnan(pack->HDOP)) {
    nmea_INFO_set_present(&pack->present, HDOP);
  }
  if (!isnan(pack->VDOP)) {
    nmea_INFO_set_present(&pack->present, VDOP);
  }

  return 1;
}

/**
 * Parse a GPGSV sentence from a string
 *
 * @param s the string
 * @param len the length of the string
 * @param has_checksum true when the string contains a checksum
 * @param pack a pointer to the result structure
 * @return 1 (true) - if parsed successfully or 0 (false) otherwise.
 */
int nmea_parse_GPGSV(const char *s, const int len, bool has_checksum, nmeaGPGSV *pack) {
  int token_count;
  int token_count_expected;
  int sat_count;
  int sat_counted = 0;

  if (!has_checksum) {
    return 0;
  }

  assert(s);
  assert(pack);

  nmea_trace_buff(s, len);

  /*
   * Clear before parsing, to be able to detect absent fields
   */
  memset(pack, 0, sizeof(nmeaGPGSV));


  /* parse */
  token_count = nmea_scanf(s, len, "$%*%*GSV,%d,%d,%d,"
      "%d,%d,%d,%d,"
      "%d,%d,%d,%d,"
      "%d,%d,%d,%d,"
      "%d,%d,%d,%d*", &pack->pack_count, &pack->pack_index, &pack->sat_count, &pack->sat_data[0].id,
      &pack->sat_data[0].elv, &pack->sat_data[0].azimuth, &pack->sat_data[0].sig, &pack->sat_data[1].id,
      &pack->sat_data[1].elv, &pack->sat_data[1].azimuth, &pack->sat_data[1].sig, &pack->sat_data[2].id,
      &pack->sat_data[2].elv, &pack->sat_data[2].azimuth, &pack->sat_data[2].sig, &pack->sat_data[3].id,
      &pack->sat_data[3].elv, &pack->sat_data[3].azimuth, &pack->sat_data[3].sig);

  /* return if we have no sentences or sats */
  if ((pack->pack_count < 1) || (pack->pack_count > NMEA_NSATPACKS) || (pack->pack_index < 1)
      || (pack->pack_index > pack->pack_count) || (pack->sat_count < 0) || (pack->sat_count > NMEA_MAXSAT)) {
    nmea_error("GPGSV parse error: inconsistent pack (count/index/satcount = %d/%d/%d)", pack->pack_count,
        pack->pack_index, pack->sat_count);
    return 0;
  }

  /* validate all sat settings and count the number of sats in the sentence */
  for (sat_count = 0; sat_count < NMEA_SATINPACK; sat_count++) {
    if (pack->sat_data[sat_count].id != 0) {
      if ((pack->sat_data[sat_count].id < 0)) {
        nmea_error("GPGSV parse error: invalid sat %d id (%d)", sat_count + 1, pack->sat_data[sat_count].id);
        return 0;
      }
      if ((pack->sat_data[sat_count].elv < -90) || (pack->sat_data[sat_count].elv > 90)) {
        nmea_error("GPGSV parse error: invalid sat %d elevation (%d)", sat_count + 1, pack->sat_data[sat_count].elv);
        return 0;
      }
      if ((pack->sat_data[sat_count].azimuth < 0) || (pack->sat_data[sat_count].azimuth >= 360)) {
        nmea_error("GPGSV parse error: invalid sat %d azimuth (%d)", sat_count + 1, pack->sat_data[sat_count].azimuth);
        return 0;
      }
      if ((pack->sat_data[sat_count].sig < 0) || (pack->sat_data[sat_count].sig > 99)) {
        nmea_error("GPGSV parse error: invalid sat %d signal (%d)", sat_count + 1, pack->sat_data[sat_count].sig);
        return 0;
      }
      sat_counted++;
    }
  }

  /* see that we have enough tokens */
  token_count_expected = (sat_counted * 4) + 3;
  if ((token_count < token_count_expected) || (token_count > (NMEA_SATINPACK * 4 + 3))) {
    nmea_error("GPGSV parse error: need %d tokens, got %d", token_count_expected, token_count);
    return 0;
  }

  /* determine which fields are present and validate them */

  if (pack->sat_count > 0) {
    nmea_INFO_set_present(&pack->present, SATINVIEW);
  }

  return 1;
}

/**
 * Parse a GPRMC sentence from a string
 *
 * @param s the string
 * @param len the length of the string
 * @param has_checksum true when the string contains a checksum
 * @param pack a pointer to the result structure
 * @return 1 (true) - if parsed successfully or 0 (false) otherwise.
 */
int nmea_parse_GPRMC(const char *s, const int len, bool has_checksum, nmeaGPRMC *pack) {
  int token_count;
  char time_buff[NMEA_TIMEPARSE_BUF];
  int date;
  size_t time_buff_len = 0;

  if (!has_checksum) {
    return 0;
  }

  assert(s);
  assert(pack);

  nmea_trace_buff(s, len);

  /*
   * Clear before parsing, to be able to detect absent fields
   */
  time_buff[0] = '\0';
  date = -1;
  pack->present = 0;
  pack->utc.year = -1;
  pack->utc.mon = -1;
  pack->utc.day = -1;
  pack->utc.hour = -1;
  pack->utc.min = -1;
  pack->utc.sec = -1;
  pack->utc.hsec = -1;
  pack->status = 0;
  pack->lat = NAN;
  pack->ns = 0;
  pack->lon = NAN;
  pack->ew = 0;
  pack->speed = NAN;
  pack->track = NAN;
  pack->magvar = NAN;
  pack->magvar_ew = 0;
  pack->mode = 0;


  /* parse */
  token_count = nmea_scanf(s, len, "$%*%*RMC,%s,%c,%f,%c,%f,%c,%f,%f,%d,%f,%c,%c*",&time_buff[0], &pack->status,
      &pack->lat, &pack->ns, &pack->lon, &pack->ew, &pack->speed, &pack->track, &date,
      &pack->magvar, &pack->magvar_ew, &pack->mode);

  /* see that we have enough tokens */
  if ((token_count != 11) && (token_count != 12)) {
    nmea_error("GPRMC parse error: need 11 or 12 tokens, got %d in %s", token_count, s);
    return 0;
  }

  /* determine which fields are present and validate them */

  time_buff_len = strlen(&time_buff[0]);
  if (time_buff_len) {
    if (!_nmea_parse_time(&time_buff[0], time_buff_len, &pack->utc)) {
      return 0;
    }

    if (!validateTime(&pack->utc)) {
      return 0;
    }

    nmea_INFO_set_present(&pack->present, UTCTIME);
  }

  if (!pack->status) {
    pack->status = 'V';
  } else {
    pack->status = toupper(pack->status);
    if (!((pack->status == 'A') || (pack->status == 'V'))) {
      nmea_error("GPRMC parse error: invalid status (%c)", pack->status);
      return 0;
    }
  }
  if (!isnan(pack->lat) && (pack->ns)) {
    if (!validateNSEW(&pack->ns, true)) {
      return 0;
    }

    nmea_INFO_set_present(&pack->present, LAT);
  }
  if (!isnan(pack->lon) && (pack->ew)) {
    if (!validateNSEW(&pack->ew, false)) {
      return 0;
    }

    nmea_INFO_set_present(&pack->present, LON);
  }
  if (!isnan(pack->speed)) {
    nmea_INFO_set_present(&pack->present, SPEED);
  }
  if (!isnan(pack->track)) {
    nmea_INFO_set_present(&pack->present, TRACK);
  }

  if (date != -1) {
    if (!_nmea_parse_date(date, &pack->utc)) {
      return 0;
    }

    if (!validateDate(&pack->utc)) {
      return 0;
    }

    nmea_INFO_set_present(&pack->present, UTCDATE);
  }

  if (!isnan(pack->magvar) && (pack->magvar_ew)) {
    if (!validateNSEW(&pack->magvar_ew, false)) {
      return 0;
    }

    nmea_INFO_set_present(&pack->present, MAGVAR);
  }
  if (token_count == 11) {
    pack->mode = 'A';
  } else {
    if (!pack->mode) {
      pack->mode = 'N';
    } else {
      if (!validateMode(&pack->mode)) {
        return 0;
      }
    }
  }

  return 1;
}

/**
 * Parse a GPVTG sentence from a string
 *
 * @param s the string
 * @param len the length of the string
 * @param has_checksum true when the string contains a checksum
 * @param pack a pointer to the result structure
 * @return 1 (true) - if parsed successfully or 0 (false) otherwise.
 */
int nmea_parse_GPVTG(const char *s, const int len, bool has_checksum, nmeaGPVTG *pack) {
  int token_count;

  if (!has_checksum) {
    return 0;
  }

  assert(s);
  assert(pack);

  nmea_trace_buff(s, len);

  /*
   * Clear before parsing, to be able to detect absent fields
   */
  pack->present = 0;
  pack->track = NAN;
  pack->track_t = 0;
  pack->mtrack = NAN;
  pack->mtrack_m = 0;
  pack->spn = NAN;
  pack->spn_n = 0;
  pack->spk = NAN;
  pack->spk_k = 0;


  /* parse */
  token_count = nmea_scanf(s, len, "$%*%*VTG,%f,%c,%f,%c,%f,%c,%f,%c*",&pack->track, &pack->track_t, &pack->mtrack,
      &pack->mtrack_m, &pack->spn, &pack->spn_n, &pack->spk, &pack->spk_k);

  /* see that we have enough tokens */
  if (token_count != 8) {
    nmea_error("GPVTG parse error: need 8 tokens, got %d in %s", token_count, s);
    return 0;
  }

  /* determine which fields are present and validate them */

  if (!isnan(pack->track) && (pack->track_t)) {
    pack->track_t = toupper(pack->track_t);
    if (pack->track_t != 'T') {
      nmea_error("GPVTG parse error: invalid track unit, got %c, expected T", pack->track_t);
      return 0;
    }

    nmea_INFO_set_present(&pack->present, TRACK);
  }
  if (!isnan(pack->mtrack) && (pack->mtrack_m)) {
    pack->mtrack_m = toupper(pack->mtrack_m);
    if (pack->mtrack_m != 'M') {
      nmea_error("GPVTG parse error: invalid mtrack unit, got %c, expected M", pack->mtrack_m);
      return 0;
    }

    nmea_INFO_set_present(&pack->present, MTRACK);
  }
  if (!isnan(pack->spn) && (pack->spn_n)) {
    pack->spn_n = toupper(pack->spn_n);
    if (pack->spn_n != 'N') {
      nmea_error("GPVTG parse error: invalid knots speed unit, got %c, expected N", pack->spn_n);
      return 0;
    }

    nmea_INFO_set_present(&pack->present, SPEED);

    if (isnan(pack->spk)) {
      pack->spk = pack->spn * NMEA_TUD_KNOTS;
      pack->spk_k = 'K';
    }
  }
  if (!isnan(pack->spk) && (pack->spk_k)) {
    pack->spk_k = toupper(pack->spk_k);
    if (pack->spk_k != 'K') {
      nmea_error("GPVTG parse error: invalid kph speed unit, got %c, expected K", pack->spk_k);
      return 0;
    }

    nmea_INFO_set_present(&pack->present, SPEED);

    if (isnan(pack->spn)) {
      pack->spn = pack->spk / NMEA_TUD_KNOTS;
      pack->spn_n = 'N';
    }
  }

  return 1;
}

/**
 * Parse a PRTI01 sentence from a string
 *
 * @param s the string
 * @param len the length of the string
 * @param has_checksum true when the string contains a checksum
 * @param pack a pointer to the result structure
 * @return 1 (true) - if parsed successfully or 0 (false) otherwise.
 */
int nmea_parse_PRTI01(const char *s, const int len, bool has_checksum, nmeaPRTI01 *pack) {
  int token_count;

  if (!has_checksum) {
    return 0;
  }

  assert(s);
  assert(pack);

  nmea_trace_buff(s, len);

  /*
   * Clear before parsing, to be able to detect absent fields
   */

  pack->present   = NAN;     /**< Mask specifying which fields are present, same as in nmeaINFO */
  pack->utc       = NAN;          /**< UTC of sample */
  pack->sample_nb = NAN;        /**< Sample number */
  pack->temp      = NAN;         /**< Temperature in hundreds of degrees Celsius. Measured at the transducer*/
  pack->bottom_vx = NAN;      /**< Bottom track X velocity component mm/s. -99999 indicates no valid velocity*/
  pack->bottom_vy = NAN;      /**< Bottom track Y velocity component mm/s. -99999 indicates no valid velocity*/
  pack->bottom_vz = NAN;      /**< Bottom track Z velocity component mm/s. -99999 indicates no valid velocity*/
  pack->bottom_depth    = NAN;     /**< Depth below transducer in mm. Range to the bottom, 0 = no detection*/
  pack->water_vx  = NAN;       /**< Water mass X velocity component mm/s. -99999 indicates no valid velocity*/
  pack->water_vy  = NAN;       /**< Water mass Y velocity component mm/s. -99999 indicates no valid velocity*/
  pack->water_vz  = NAN;       /**< Water mass Z velocity component mm/s. -99999 indicates no valid velocity*/
  pack->water_depth   = NAN;      /**< Depth of water mass measurement in mm. Position of bin in front of the transducer*/
  strcpy(pack->cbit, "0000");       /**< Built in test and status bits in hexadecimal0000 = OK*/
  pack->S   = 0;            /**< Sub System*/
  pack->N   = 0;            /**< Sub System index*/

   /* parse */
  //$PRTI01,sssssshh,nnnnnn,TTTT,XXXX,YYYY,ZZZZ,DDDD,xxxx,yyyy,zzzz,dddd,ABCD,S,N
  token_count = nmea_scanf(s, len, "$PRTI01,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%s,%i,%i*",
   &pack->utc, &pack->sample_nb, &pack->temp,
   &pack->bottom_vx, &pack->bottom_vy, &pack->bottom_vz, &pack->bottom_depth, 
   &pack->water_vx, &pack->water_vy, &pack->water_vz, &pack->water_depth, 
   &pack->cbit, &pack->S, &pack->N);

  /* see that we have enough tokens */
  if (token_count != 14) {
    nmea_error("PRTI01 parse error: need 14 tokens, got %d in %s", token_count, s);
    return 0;
  }

  if (!isnan(pack->bottom_vx) && !isnan(pack->bottom_vy) && !isnan(pack->bottom_vz)) {
    nmea_INFO_set_present(&pack->present, RTI);
  }

  return 1;
}
