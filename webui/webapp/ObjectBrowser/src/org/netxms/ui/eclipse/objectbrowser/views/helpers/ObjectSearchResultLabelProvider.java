/**
 * 
 */
package org.netxms.ui.eclipse.objectbrowser.views.helpers;

import org.eclipse.jface.viewers.ITableLabelProvider;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.swt.graphics.Image;
import org.eclipse.ui.model.WorkbenchLabelProvider;
import org.netxms.base.InetAddressEx;
import org.netxms.client.objects.AbstractNode;
import org.netxms.client.objects.AbstractObject;
import org.netxms.client.objects.AccessPoint;
import org.netxms.client.objects.Interface;
import org.netxms.client.objects.Zone;
import org.netxms.ui.eclipse.objectbrowser.views.ObjectFinder;
import org.netxms.ui.eclipse.shared.ConsoleSharedData;

/**
 * Label provider for object search results
 */
public class ObjectSearchResultLabelProvider extends LabelProvider implements ITableLabelProvider
{
   WorkbenchLabelProvider wbLabelProvider = new WorkbenchLabelProvider();
   
   /* (non-Javadoc)
    * @see org.eclipse.jface.viewers.ITableLabelProvider#getColumnImage(java.lang.Object, int)
    */
   @Override
   public Image getColumnImage(Object element, int columnIndex)
   {
      return (columnIndex == 0) ? wbLabelProvider.getImage(element) : null;
   }

   /* (non-Javadoc)
    * @see org.eclipse.jface.viewers.ITableLabelProvider#getColumnText(java.lang.Object, int)
    */
   @Override
   public String getColumnText(Object element, int columnIndex)
   {
      if (!(element instanceof AbstractObject))
         return null;
      AbstractObject object = (AbstractObject)element;
      switch(columnIndex)
      {
         case ObjectFinder.COL_CLASS:
            return object.getObjectClassName();
         case ObjectFinder.COL_ID:
            return Long.toString(object.getObjectId());
         case ObjectFinder.COL_IP_ADDRESS:
            if (object instanceof AbstractNode)
            {
               InetAddressEx addr = ((AbstractNode)object).getPrimaryIP();
               return addr.isValidUnicastAddress() ? addr.getHostAddress() : null;
            }
            if (object instanceof AccessPoint)
            {
               InetAddressEx addr = ((AccessPoint)object).getIpAddress();
               return addr.isValidUnicastAddress() ? addr.getHostAddress() : null;
            }
            if (object instanceof Interface)
            {
               return ((Interface)object).getIpAddressListAsString();
            }
            return null;
         case ObjectFinder.COL_NAME:
            return wbLabelProvider.getText(element);
         case ObjectFinder.COL_ZONE:
            if (object instanceof AbstractNode)
            {
               long zoneId = ((AbstractNode)object).getZoneId();
               Zone zone = ConsoleSharedData.getSession().findZone(zoneId);
               return (zone != null) ? zone.getObjectName() + " (" + Long.toString(zoneId) + ")" : Long.toString(zoneId); 
            }
            return null;
      }
      return null;
   }

   /* (non-Javadoc)
    * @see org.eclipse.jface.viewers.BaseLabelProvider#dispose()
    */
   @Override
   public void dispose()
   {
      wbLabelProvider.dispose();
      super.dispose();
   }
}
