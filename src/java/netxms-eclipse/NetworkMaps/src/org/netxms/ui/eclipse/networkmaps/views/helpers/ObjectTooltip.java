/**
 * NetXMS - open source network management system
 * Copyright (C) 2003-2020 Victor Kirhenshtein
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
package org.netxms.ui.eclipse.networkmaps.views.helpers;

import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.draw2d.Figure;
import org.eclipse.draw2d.GridData;
import org.eclipse.draw2d.GridLayout;
import org.eclipse.draw2d.Label;
import org.eclipse.draw2d.LineBorder;
import org.eclipse.draw2d.MarginBorder;
import org.eclipse.draw2d.text.FlowPage;
import org.eclipse.draw2d.text.TextFlow;
import org.eclipse.jface.resource.JFaceResources;
import org.eclipse.swt.SWT;
import org.netxms.base.MacAddress;
import org.netxms.client.NXCSession;
import org.netxms.client.constants.DataOrigin;
import org.netxms.client.constants.DataType;
import org.netxms.client.constants.ObjectStatus;
import org.netxms.client.datacollection.DciValue;
import org.netxms.client.datacollection.GraphItem;
import org.netxms.client.datacollection.GraphSettings;
import org.netxms.client.maps.MapObjectDisplayMode;
import org.netxms.client.objects.AbstractNode;
import org.netxms.client.objects.AbstractObject;
import org.netxms.client.objects.AccessPoint;
import org.netxms.client.objects.Container;
import org.netxms.client.objects.Node;
import org.netxms.client.topology.RadioInterface;
import org.netxms.ui.eclipse.charts.api.ChartColor;
import org.netxms.ui.eclipse.charts.figures.BirtChartFigure;
import org.netxms.ui.eclipse.console.resources.SharedIcons;
import org.netxms.ui.eclipse.console.resources.StatusDisplayInfo;
import org.netxms.ui.eclipse.jobs.ConsoleJob;
import org.netxms.ui.eclipse.networkmaps.Activator;
import org.netxms.ui.eclipse.networkmaps.Messages;
import org.netxms.ui.eclipse.shared.ConsoleSharedData;

/**
 * Tooltip for object on map
 */
public class ObjectTooltip extends Figure
{
	private NodeLastValuesFigure lastValuesFigure = null;
   private int index;
   private AbstractObject object;
   private MapLabelProvider labelProvider;

	/**
	 * @param object
	 */
	public ObjectTooltip(AbstractObject object, MapLabelProvider labelProvider)
	{
	   this.object = object;
	   this.labelProvider = labelProvider;
	   
		setBorder(new MarginBorder(3));
		GridLayout layout = new GridLayout(2, false);
		layout.horizontalSpacing = 10;
		setLayoutManager(layout);

		Label title = new Label();
		title.setIcon(labelProvider.getWorkbenchIcon(object));
		title.setText(object.getObjectName());
		title.setFont(JFaceResources.getBannerFont());
		add(title);

		Label status = new Label();
		status.setIcon(StatusDisplayInfo.getStatusImage(object.getStatus()));
		status.setText(StatusDisplayInfo.getStatusText(object.getStatus()));
		add(status);
		GridData gd = new GridData();
		gd.horizontalAlignment = SWT.RIGHT;
		setConstraint(status, gd);

      if (object instanceof Node)
      {
         if ((((Node)object).getCapabilities() & AbstractNode.NC_IS_ETHERNET_IP) != 0)
         {
            if ((((Node)object).getHardwareProductName() != null) && !((Node)object).getHardwareProductName().isEmpty())
               addInformationBlock(((Node)object).getHardwareProductName() + " (" + ((Node)object).getCipDeviceTypeName() + ")", null);
            else
               addInformationBlock(((Node)object).getCipDeviceTypeName(), null);
         }
         else
         {
            addInformationBlock(((Node)object).getHardwareProductName(), null);
         }
         addInformationBlock(((Node)object).getHardwareVendor(), "Vendor: ");
      }

		if ((object instanceof Node) && ((Node)object).getPrimaryIP().isValidAddress() && !((Node)object).getPrimaryIP().getAddress().isAnyLocalAddress())
		{
			StringBuilder sb = new StringBuilder(((Node)object).getPrimaryIP().getHostAddress());
			MacAddress mac = ((Node)object).getPrimaryMAC();
			if (mac != null)
			{
				sb.append(" ("); //$NON-NLS-1$
				sb.append(mac.toString());
				sb.append(')');
			}

			Label iface = new Label();
			iface.setIcon(SharedIcons.IMG_IP_ADDRESS);
			iface.setText(sb.toString());
			add(iface);

			gd = new GridData();
			gd.horizontalSpan = 2;
			setConstraint(iface, gd);
		}
		
      index = getChildren().size();		
		if (object instanceof Node && labelProvider.getObjectFigureType() == MapObjectDisplayMode.LARGE_LABEL)
		{
   		DciValue[] values = labelProvider.getNodeLastValues(object.getObjectId());
   		if ((values != null) && (values.length > 0))
   		{
      		lastValuesFigure = new NodeLastValuesFigure(values);
            add(lastValuesFigure);
            
             gd = new GridData();
            gd.horizontalSpan = 2;
            setConstraint(lastValuesFigure, gd);
         }
      }

		if (object instanceof Container)
			addStatusChart(object, labelProvider);

		if (object instanceof AccessPoint)
		{
			StringBuilder sb = new StringBuilder(((AccessPoint)object).getModel());
			MacAddress mac = ((AccessPoint)object).getMacAddress();
			if (mac != null)
			{
				sb.append('\n');
				sb.append(mac.toString());
			}

			for(RadioInterface rif : ((AccessPoint)object).getRadios())
			{
				sb.append(Messages.get().ObjectTooltip_Radio);
				sb.append(rif.getIndex());
				sb.append(" ("); //$NON-NLS-1$
				sb.append(rif.getMacAddress().toString());
				sb.append(Messages.get().ObjectTooltip_Channel);
				sb.append(rif.getChannel());
				sb.append(Messages.get().ObjectTooltip_TXPower);
				sb.append(rif.getPowerMW());
				sb.append(Messages.get().ObjectTooltip_mW);
			}

			Label info = new Label();
			info.setText(sb.toString());
			add(info);

			gd = new GridData();
			gd.horizontalSpan = 2;
			setConstraint(info, gd);
		}

		if (!object.getComments().isEmpty())
		{
			FlowPage page = new FlowPage();
			add(page);
			gd = new GridData();
			gd.horizontalSpan = 2;
			setConstraint(page, gd);

			TextFlow text = new TextFlow();
			text.setText("\n" + object.getComments()); //$NON-NLS-1$
			page.add(text);
		}
	}
	
	public void updateLastValues()
	{
	   if (labelProvider.getObjectFigureType() == MapObjectDisplayMode.LARGE_LABEL)
	   {
	      return;
	   }
	   
      final NXCSession session = ConsoleSharedData.getSession();
      final long nodeId = object.getObjectId();
      ConsoleJob job = new ConsoleJob("", null, Activator.PLUGIN_ID, null) {
         @Override
         protected void runInternal(IProgressMonitor monitor) throws Exception
         {
            try
            {
               final DciValue[] values = session.getLastValues(nodeId, false, true, false);
               runInUIThread(new Runnable() {
                  @Override
                  public void run()
                  {
                     if (object instanceof Node)
                     {
                        if ((values != null) && (values.length > 0))
                        {
                           if (lastValuesFigure != null)
                              remove(lastValuesFigure);
                           lastValuesFigure = new NodeLastValuesFigure(values);
                           GridData gd = new GridData();
                           gd.horizontalSpan = 2;
                           System.out.println(index);
                           add(lastValuesFigure, gd, index);
                           layout();
                           invalidateTree();
                           revalidate();
                           repaint();
                        }
                     }                     
                  }
               });
            }
            catch(Exception e)
            {
               //Activator.log("Exception in last values overview element", e); //$NON-NLS-1$
            }
         }
         
         @Override
         protected String getErrorMessage()
         {
            return "";
         }
      };
      job.setUser(false);
      job.setSystem(true);
      job.start();
	}

   /**
    * Add additional information block
    * 
    * @param text
    */
   private void addInformationBlock(String text, String prefix)
   {
      if ((text == null) || text.isEmpty())
         return;

      Label info = new Label();
      info.setText((prefix != null) ? (prefix + text) : text);
      add(info);

      GridData gd = new GridData();
      gd.horizontalSpan = 2;
      setConstraint(info, gd);
   }

	/**
	 * Status chart
	 */
	private void addStatusChart(AbstractObject object, MapLabelProvider labelProvider)
	{
		BirtChartFigure chart = new BirtChartFigure(BirtChartFigure.BAR_CHART, labelProvider.getColors());
		add(chart);
		GridData gd = new GridData();
		gd.horizontalSpan = 2;
		gd.horizontalAlignment = SWT.FILL;
		gd.heightHint = 180;
		gd.widthHint = 320;
		setConstraint(chart, gd);

		int[] objectCount = new int[6];
		collectData(objectCount, object);

		chart.setTitleVisible(true);
		chart.setChartTitle(Messages.get().ObjectTooltip_ChartTitle);
		chart.setLegendPosition(GraphSettings.POSITION_RIGHT);
		chart.setLegendVisible(true);
		chart.set3DModeEnabled(true);
		chart.setTransposed(false);
		chart.setTranslucent(false);
		chart.setBorder(new LineBorder());

		for(int i = 0; i <= ObjectStatus.UNKNOWN.getValue(); i++)
		{
         chart.addParameter(new GraphItem(0, 0, DataOrigin.INTERNAL, DataType.INT32, StatusDisplayInfo.getStatusText(i),
               StatusDisplayInfo.getStatusText(i), "%s"), objectCount[i]);
			chart.setPaletteEntry(i, new ChartColor(StatusDisplayInfo.getStatusColor(i).getRGB()));
		}
		chart.initializationComplete();
	}

	/**
	 * @param objectCount
	 * @param root
	 */
	private void collectData(int[] objectCount, AbstractObject root)
	{
		for(AbstractObject o : root.getAllChildren(AbstractObject.OBJECT_NODE))
		{
			if (o.getStatus().compareTo(ObjectStatus.UNKNOWN) <= 0)
				objectCount[o.getStatus().getValue()]++;
		}
	}
}
