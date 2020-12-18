/**
 * NetXMS - open source network management system
 * Copyright (C) 2020 Raden Solutions
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
package org.netxms.client.datacollection;

import org.eclipse.swt.events.VerifyListener;
import org.netxms.base.NXCPMessage;

/**
 * Class that contains information about edited element
 */
public class BulkUpdateElement
{
   private String name;
   private String[] possibleValues;
   private String valueS;
   private Integer valueI;
   private VerifyListener listener;
   private long fieldId;
   
   public BulkUpdateElement(String name, long fieldId, String[] possibleValues, VerifyListener listener)
   {
      this.name = name;
      this.fieldId = fieldId;
      this.possibleValues = possibleValues;
      this.listener = listener;
      valueS = "";
      valueI = 0;
   }
   
   /**
    * @return the value
    */
   public String getTextValue()
   {
      return valueS;
   }
   
   /**
    * @return the value
    */
   public Integer getSelectionValue()
   {
      return valueI;
   }
   
   /**
    * @param value2 the value to set
    */
   public void setValue(Object value2)
   {
      System.out.println("Set value "+ value2);
      if (isText())
         valueS = (String)value2;
      else 
      {
         valueI = (Integer)value2;
      }
   }
   
   /**
    * @return the name
    */
   public String getName()
   {
      return name;
   }
   /**
    * @return the possibleValues
    */
   public String[] getPossibleValues()
   {
      return possibleValues;
   }
   
   /**
    * If editable value is text
    * 
    * @return true if editable value is test value
    */
   public boolean isText()
   {
      return possibleValues == null;
   }

   /**
    * Returns if filed is modified
    * 
    * @return true if field is modified
    */
   public boolean isModified()
   {
      boolean isModified = false;
      if(isText())
         isModified = !getTextValue().isEmpty();
      else 
         isModified = getSelectionValue() != 0; 
      return isModified;
   }

   /**
    * Verify listener
    * 
    * @return verify listener for this value
    */
   public VerifyListener getVerifyListener()
   {
      return listener;
   }

   public void setField(NXCPMessage msg)
   {
      if (!isModified())
         return;

      if (isText())
      {
         msg.setField(fieldId, valueS);         
      }
      else
      {
         msg.setFieldInt32(fieldId, valueI - 1);         
      }
   }
   
   
}
