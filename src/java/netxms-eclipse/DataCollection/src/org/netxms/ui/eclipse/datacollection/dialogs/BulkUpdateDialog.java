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
package org.netxms.ui.eclipse.datacollection.dialogs;

import java.util.ArrayList;
import java.util.List;
import org.eclipse.jface.dialogs.Dialog;
import org.eclipse.jface.viewers.ArrayContentProvider;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.jface.viewers.TableViewerColumn;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Shell;
import org.netxms.base.NXCPCodes;
import org.netxms.client.datacollection.BulkUpdateElement;
import org.netxms.ui.eclipse.datacollection.Messages;
import org.netxms.ui.eclipse.datacollection.dialogs.helpers.BulkUpdateLabelProvider;
import org.netxms.ui.eclipse.datacollection.dialogs.helpers.BulkValueEditSupport;

/**
 * DCI bulk update
 */
public class BulkUpdateDialog extends Dialog
{
   private TableViewer tableViewer;
   private List<BulkUpdateElement> element;

   public BulkUpdateDialog(Shell parentShell)
   {      
      super(parentShell);
      element = new ArrayList<BulkUpdateElement>();
      String[] pollingModes = {"No change", Messages.get().General_FixedIntervalsDefault, Messages.get().General_FixedIntervalsCustom, Messages.get().General_CustomSchedule};
      element.add(new BulkUpdateElement("Polling mode", NXCPCodes.VID_POLLING_SCHEDULE_TYPE, pollingModes, null));
      element.add(new BulkUpdateElement("Polling interval (seconds)", NXCPCodes.VID_POLLING_INTERVAL, null, null));
      
      String[] retentionModes = {"No change", Messages.get().General_UseDefaultRetention, Messages.get().General_UseCustomRetention, Messages.get().General_NoStorage};
      element.add(new BulkUpdateElement("Retention mode", NXCPCodes.VID_RETENTION_TYPE, retentionModes, null));
      element.add(new BulkUpdateElement("Retention time (days)", NXCPCodes.VID_RETENTION_TIME, null, null));
   }

   /* (non-Javadoc)
    * @see org.eclipse.jface.dialogs.Dialog#createDialogArea(org.eclipse.swt.widgets.Composite)
    */
   @Override
   protected Control createDialogArea(Composite parent)
   {
      Composite dialogArea = (Composite)super.createDialogArea(parent);

      int style = SWT.BORDER | SWT.H_SCROLL | SWT.V_SCROLL | SWT.HIDE_SELECTION;      

      String[] columnNames = new String[] { "Name", "Value" };
      tableViewer = new TableViewer(parent, style);
      
      tableViewer.setColumnProperties(columnNames);  
      

      TableViewerColumn column = new TableViewerColumn(tableViewer, SWT.LEFT);
      column.getColumn().setText("Name");
      column.getColumn().setWidth(300);
      

      column = new TableViewerColumn(tableViewer, SWT.LEFT);
      column.getColumn().setText("Value");
      column.getColumn().setWidth(400);
      column.setEditingSupport(new BulkValueEditSupport(tableViewer));  
      

      tableViewer.getTable().setLinesVisible(true);
      tableViewer.getTable().setHeaderVisible(true);
      tableViewer.setContentProvider(new ArrayContentProvider());
      tableViewer.setLabelProvider(new BulkUpdateLabelProvider());
      tableViewer.setInput(element.toArray());
      
      return dialogArea;
   }

   /* (non-Javadoc)
    * @see org.eclipse.jface.window.Window#configureShell(org.eclipse.swt.widgets.Shell)
    */
   @Override
   protected void configureShell(Shell newShell)
   {
      super.configureShell(newShell);
      newShell.setText("Bulk DCI update");
   }
   
   /* (non-Javadoc)
    * @see org.eclipse.jface.dialogs.Dialog#okPressed()
    */
   @Override
   protected void okPressed()
   {
      super.okPressed();
   }
   
   /**
    * Get bulk update elements
    * 
    * @return list of bulk update elements
    */
   public List<BulkUpdateElement> getBulkUpdateElements()
   {
      return element;
   }
}
