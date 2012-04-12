/**
 * NetXMS - open source network management system
 * Copyright (C) 2003-2011 Victor Kirhenshtein
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
package org.netxms.client;

import java.text.NumberFormat;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.netxms.base.NXCPCodes;
import org.netxms.base.NXCPMessage;

/**
 * Geolocation encoding
 */
public class GeoLocation
{
	private static final double ROUND_OFF = 0.00000001;

	public static final int UNSET = 0;
	public static final int MANUAL = 1;
	public static final int GPS = 2;
	
	private int type;
	private double latitude;
	private double longitude;
	
	/**
	 * Create geolocation object from NXCP message
	 * @param msg NXCP message
	 */
	public GeoLocation(final NXCPMessage msg)
	{
		type = msg.getVariableAsInteger(NXCPCodes.VID_GEOLOCATION_TYPE);
		latitude = msg.getVariableAsReal(NXCPCodes.VID_LATITUDE);
		longitude = msg.getVariableAsReal(NXCPCodes.VID_LONGITUDE);
	}
	
	/**
	 * Create geolocation object of type UNSET or GPS
	 */
	public GeoLocation(boolean isGPS)
	{
		type = isGPS ? GPS : UNSET;
		latitude = 0.0;
		longitude = 0.0;
	}
	
	/**
	 * Create geolocation object of type MANUAL
	 * 
	 * @param lat Latitude
	 * @param lon Longitude
	 */
	public GeoLocation(double lat, double lon)
	{
		type = MANUAL;
		latitude = lat;
		longitude = lon;
	}

	/**
	 * @return the type
	 */
	public int getType()
	{
		return type;
	}

	/**
	 * @return the latitude
	 */
	public double getLatitude()
	{
		return latitude;
	}

	/**
	 * @return the longitude
	 */
	public double getLongitude()
	{
		return longitude;
	}
	
	/**
	 * Extract degrees from lat/lon value
	 * 
	 * @param pos
	 * @return
	 */
	private static int getIntegerDegree(double pos)
	{
		return (int)(Math.abs(pos) + ROUND_OFF);
	}
	
	/**
	 * Extract minutes from lat/lon value
	 * 
	 * @param pos
	 * @return
	 */
	private static int getIntegerMinutes(double pos)
	{
		double d = Math.abs(pos) + ROUND_OFF;
		return (int)((d - (double)((int)d)) * 60.0);
	}
	
	/**
	 * Extract seconds from lat/lon value
	 * 
	 * @param pos
	 * @return
	 */
	private static double getDecimalSeconds(double pos)
	{
		double d = Math.abs(pos) * 60.0 + ROUND_OFF;
		return (d - (double)((int)d)) * 60.0;
	}

	/**
	 * Convert latitude or longitude to text
	 * 
	 * @param pos position
	 * @param isLat true if given position is latitude
	 * @return
	 */
	private static String posToText(double pos, boolean isLat)
	{
		if ((pos < -180.0) || (pos > 180.0))
			return "<invalid>";
		
		StringBuilder sb = new StringBuilder();

		// Encode hemisphere
		if (isLat)
		{
			sb.append((pos < 0) ? 'S' : 'N');
		}
		else
		{
			sb.append((pos < 0) ? 'W' : 'E');
		}
		sb.append(' ');
		
		NumberFormat nf = NumberFormat.getIntegerInstance();
		nf.setMinimumIntegerDigits(2);
		sb.append(nf.format(getIntegerDegree(pos)));
		sb.append("\u00b0 ");
		sb.append(nf.format(getIntegerMinutes(pos)));
		sb.append("' ");

		nf = NumberFormat.getNumberInstance();
		nf.setMinimumIntegerDigits(2);
		nf.setMinimumFractionDigits(3);
		nf.setMaximumFractionDigits(3);
		sb.append(nf.format(getDecimalSeconds(pos)));
		sb.append('"');
		
		return sb.toString();
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString()
	{
		return posToText(latitude, true) + " " + posToText(longitude, false);
	}
	
	/**
	 * Convert latitude from floating point to text representation
	 * 
	 * @param lat latitude
	 * @return text representation of given latitude
	 */
	public static String latitudeToString(double lat)
	{
		return posToText(lat, true);
	}
	
	/**
	 * Convert longitude from floating point to text representation
	 * 
	 * @param lon longitude
	 * @return text representation of given longitude
	 */
	public static String longitudeToString(double lon)
	{
		return posToText(lon, false);
	}
	
	/**
	 * @return latitude as DMS string
	 */
	public String getLatitudeAsString()
	{
		return posToText(latitude, true);
	}
	
	/**
	 * @return longitude as DMS string
	 */
	public String getLongitudeAsString()
	{
		return posToText(longitude, false);
	}
	
	/**
	 * Parse geolocation component - either latitude or longitude
	 * 
	 * @param str string representation of component
	 * @param isLat true if given string is a latitude
	 * @return double value for component
	 * @throws GeoLocationFormatException
	 */
	private static double parse(String str, boolean isLat) throws GeoLocationFormatException
	{
		double value;
		String in = str.trim();
		
		// Try to parse as simple double value first
		try
		{
			value = Double.parseDouble(str);
			if ((value < -180.0) || (value > 180.0))
				throw new GeoLocationFormatException();
			return value;
		}
		catch(NumberFormatException e)
		{
		}
		
		Pattern p = Pattern.compile((isLat ? "([NS]*)" : "([EW]*)") + "\\s*([0-9]+(?:\\.[0-9]+)*)\u00b0?\\s*([0-9]+(?:\\.[0-9]+)*)?\\'?\\s*([0-9]+(?:\\.[0-9]+)*)?\\\"?\\s*" + (isLat ? "([NS]*)" : "([EW]*)"));
		Matcher m = p.matcher(in);
		if (m.matches())
		{
			// Hemisphere sign
			char ch;
			if ((m.group(1) != null) && !m.group(1).isEmpty())
				ch = m.group(1).charAt(0);
			else if ((m.group(5) != null) && !m.group(5).isEmpty())
				ch = m.group(5).charAt(0);
			else
				throw new GeoLocationFormatException();
			int sign = ((ch == 'N') || (ch == 'E')) ? 1 : -1;
			
			try
			{
				double deg = Double.parseDouble(m.group(2));
				double min = (m.group(3) != null) ? Double.parseDouble(m.group(3)) : 0.0;
				double sec = (m.group(4) != null) ? Double.parseDouble(m.group(4)) : 0.0;
				
				value = sign * (deg + min / 60.0 + sec / 3600.0);
				if ((value < -180.0) || (value > 180.0))
					throw new GeoLocationFormatException();
				
				return value;
			}
			catch(NumberFormatException e)
			{
				throw new GeoLocationFormatException();
			}
		}
		else
		{
			throw new GeoLocationFormatException();
		}
	}
	
	/**
	 * Parse geolocation string. Latitude and longitude must be given either
	 * as numeric values or in DMS form.
	 * 
	 * @param lat latitude string
	 * @param lon longitude string
	 * @return geolocation object
	 * @throws GeoLocationFormatException if the strings does not contain a parsable geolocation
	 */
	public static GeoLocation parseGeoLocation(String lat, String lon) throws GeoLocationFormatException
	{
		double _lat = parse(lat, true);
		double _lon = parse(lon, false);
		return new GeoLocation(_lat, _lon);
	}
}
