/**
 * 
 */
package org.netxms.ui.eclipse.networkmaps.propertypages;

import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.preference.ColorSelector;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Spinner;
import org.eclipse.ui.dialogs.PropertyPage;
import org.netxms.client.NXCObjectModificationData;
import org.netxms.client.NXCSession;
import org.netxms.client.objects.NetworkMap;
import org.netxms.ui.eclipse.jobs.ConsoleJob;
import org.netxms.ui.eclipse.networkmaps.Activator;
import org.netxms.ui.eclipse.shared.ConsoleSharedData;
import org.netxms.ui.eclipse.tools.ColorConverter;
import org.netxms.ui.eclipse.tools.WidgetHelper;

/**
 * Map options property page
 */
public class MapOptions extends PropertyPage
{
	private NetworkMap object;
	private Button checkShowStatusIcon;
	private Button checkShowStatusFrame;
	private Button checkShowStatusBkgnd;
	private Combo routingAlgorithm;
	private Button radioColorDefault;
	private Button radioColorCustom;
	private ColorSelector linkColor;
	private Button checkIncludeEndNodes;
	private Button checkCustomRadius;
	private Spinner topologyRadius;
	private Button checkCalculateStatus;

	/* (non-Javadoc)
	 * @see org.eclipse.jface.preference.PreferencePage#createContents(org.eclipse.swt.widgets.Composite)
	 */
	@Override
	protected Control createContents(Composite parent)
	{
		Composite dialogArea = new Composite(parent, SWT.NONE);
		
		object = (NetworkMap)getElement().getAdapter(NetworkMap.class);

		GridLayout layout = new GridLayout();
		layout.verticalSpacing = WidgetHelper.OUTER_SPACING;
		layout.marginWidth = 0;
		layout.marginHeight = 0;
		layout.numColumns = 2;
		dialogArea.setLayout(layout);
		
		/**** status display ****/
		Group statusDisplayGroup = new Group(dialogArea, SWT.NONE);
		statusDisplayGroup.setText("Default display options");
      GridData gd = new GridData();
      gd.grabExcessHorizontalSpace = true;
      gd.horizontalAlignment = SWT.FILL;
      gd.verticalAlignment = SWT.FILL;
      statusDisplayGroup.setLayoutData(gd);
      layout = new GridLayout();
      statusDisplayGroup.setLayout(layout);
      
      checkShowStatusIcon = new Button(statusDisplayGroup, SWT.CHECK);
      checkShowStatusIcon.setText("Show status &icon");
      checkShowStatusIcon.setSelection((object.getFlags() & NetworkMap.MF_SHOW_STATUS_ICON) != 0);
      
      checkShowStatusFrame = new Button(statusDisplayGroup, SWT.CHECK);
      checkShowStatusFrame.setText("Show status &frame");
      checkShowStatusFrame.setSelection((object.getFlags() & NetworkMap.MF_SHOW_STATUS_FRAME) != 0);
      
      checkShowStatusBkgnd = new Button(statusDisplayGroup, SWT.CHECK);
      checkShowStatusBkgnd.setText("Show status &background");
      checkShowStatusBkgnd.setSelection((object.getFlags() & NetworkMap.MF_SHOW_STATUS_BKGND) != 0);
      
		/**** default link appearance ****/
		Group linkGroup = new Group(dialogArea, SWT.NONE);
		linkGroup.setText("Default connection options");
      gd = new GridData();
      gd.grabExcessHorizontalSpace = true;
      gd.horizontalAlignment = SWT.FILL;
      gd.verticalAlignment = SWT.FILL;
      linkGroup.setLayoutData(gd);
      layout = new GridLayout();
      linkGroup.setLayout(layout);

      gd = new GridData();
		gd.horizontalAlignment = SWT.FILL;
		gd.grabExcessHorizontalSpace = true;
		routingAlgorithm = WidgetHelper.createLabeledCombo(linkGroup, SWT.READ_ONLY, "Routing algorithm", gd);
		routingAlgorithm.add("Direct");
		routingAlgorithm.add("Manhattan");
		routingAlgorithm.select(object.getDefaultLinkRouting() - 1);

		final SelectionListener listener = new SelectionListener() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				linkColor.setEnabled(radioColorCustom.getSelection());
			}
			
			@Override
			public void widgetDefaultSelected(SelectionEvent e)
			{
				widgetSelected(e);
			}
		};
		
		radioColorDefault = new Button(linkGroup, SWT.RADIO);
		radioColorDefault.setText("&Default color");
		radioColorDefault.setSelection(object.getDefaultLinkColor() < 0);
		radioColorDefault.addSelectionListener(listener);
		gd = new GridData();
		gd.verticalIndent = WidgetHelper.OUTER_SPACING * 2;
		radioColorDefault.setLayoutData(gd);

		radioColorCustom = new Button(linkGroup, SWT.RADIO);
		radioColorCustom.setText("&Custom color");
		radioColorCustom.setSelection(object.getDefaultLinkColor() >= 0);
		radioColorCustom.addSelectionListener(listener);

		linkColor = new ColorSelector(linkGroup);
		linkColor.setColorValue(ColorConverter.rgbFromInt(object.getDefaultLinkColor()));
		linkColor.setEnabled(object.getDefaultLinkColor() >= 0);
		gd = new GridData();
		gd.horizontalIndent = 20;
		linkColor.getButton().setLayoutData(gd);
		
		/**** topology options ****/
      if (object.getMapType() != NetworkMap.TYPE_CUSTOM)
      {
			Group topoGroup = new Group(dialogArea, SWT.NONE);
			topoGroup.setText("Topology options");
	      gd = new GridData();
	      gd.grabExcessHorizontalSpace = true;
	      gd.horizontalAlignment = SWT.FILL;
	      gd.horizontalSpan = 2;
	      topoGroup.setLayoutData(gd);
	      layout = new GridLayout();
	      topoGroup.setLayout(layout);

	      checkIncludeEndNodes = new Button(topoGroup, SWT.CHECK);
	      checkIncludeEndNodes.setText("Include &end nodes");
	      checkIncludeEndNodes.setSelection((object.getFlags() & NetworkMap.MF_SHOW_END_NODES) != 0);
	      
	      checkCustomRadius = new Button(topoGroup, SWT.CHECK);
	      checkCustomRadius.setText("Custom discovery &radius");
	      checkCustomRadius.setSelection(object.getDiscoveryRadius() > 0);
	      checkCustomRadius.addSelectionListener(new SelectionListener() {
				@Override
				public void widgetSelected(SelectionEvent e)
				{
			      topologyRadius.setEnabled(checkCustomRadius.getSelection());
				}
				
				@Override
				public void widgetDefaultSelected(SelectionEvent e)
				{
					widgetSelected(e);
				}
			});
	      
	      topologyRadius = WidgetHelper.createLabeledSpinner(topoGroup, SWT.BORDER, "Topology discovery radius", 1, 255, WidgetHelper.DEFAULT_LAYOUT_DATA);
	      topologyRadius.setSelection(object.getDiscoveryRadius());
	      topologyRadius.setEnabled(object.getDiscoveryRadius() > 0);
      }      

		/**** advanced options ****/
		Group advGroup = new Group(dialogArea, SWT.NONE);
		advGroup.setText("Advanced options");
      gd = new GridData();
      gd.grabExcessHorizontalSpace = true;
      gd.horizontalAlignment = SWT.FILL;
      gd.horizontalSpan = 2;
      advGroup.setLayoutData(gd);
      layout = new GridLayout();
      advGroup.setLayout(layout);

      checkCalculateStatus = new Button(advGroup, SWT.CHECK);
      checkCalculateStatus.setText("&Calculate map status based on contained object status");
      checkCalculateStatus.setSelection((object.getFlags() & NetworkMap.MF_CALCULATE_STATUS) != 0);
      
		return dialogArea;
	}

	/**
	 * Apply changes
	 * 
	 * @param isApply true if update operation caused by "Apply" button
	 */
	protected boolean applyChanges(final boolean isApply)
	{
		final NXCObjectModificationData md = new NXCObjectModificationData(object.getObjectId());
		
		int flags = 0;
		if ((checkIncludeEndNodes != null) && checkIncludeEndNodes.getSelection())
			flags |= NetworkMap.MF_SHOW_END_NODES;
		if (checkShowStatusIcon.getSelection())
			flags |= NetworkMap.MF_SHOW_STATUS_ICON;
		if (checkShowStatusFrame.getSelection())
			flags |= NetworkMap.MF_SHOW_STATUS_FRAME;
		if (checkShowStatusBkgnd.getSelection())
			flags |= NetworkMap.MF_SHOW_STATUS_BKGND;
		if (checkCalculateStatus.getSelection())
			flags |= NetworkMap.MF_CALCULATE_STATUS;
		md.setObjectFlags(flags);
		
		md.setConnectionRouting(routingAlgorithm.getSelectionIndex() + 1);
		
		if (radioColorCustom.getSelection())
		{
			md.setLinkColor(ColorConverter.rgbToInt(linkColor.getColorValue()));
		}
		else
		{
			md.setLinkColor(-1);
		}
		
		if (checkCustomRadius != null)
		{
			if (checkCustomRadius.getSelection())
				md.setDiscoveryRadius(topologyRadius.getSelection());
			else
				md.setDiscoveryRadius(-1);
		}
		
		if (isApply)
			setValid(false);

		final NXCSession session = (NXCSession)ConsoleSharedData.getSession();
		new ConsoleJob("Update map options for map object " + object.getObjectName(), null, Activator.PLUGIN_ID, null) {
			@Override
			protected void runInternal(IProgressMonitor monitor) throws Exception
			{
				session.modifyObject(md);
			}

			@Override
			protected String getErrorMessage()
			{
				return "Cannot modify options for map object " + object.getObjectName();
			}

			@Override
			protected void jobFinalize()
			{
				if (isApply)
				{
					runInUIThread(new Runnable() {
						@Override
						public void run()
						{
							MapOptions.this.setValid(true);
						}
					});
				}
			}
		}.start();
		
		return true;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.preference.PreferencePage#performOk()
	 */
	@Override
	public boolean performOk()
	{
		return applyChanges(false);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.preference.PreferencePage#performApply()
	 */
	@Override
	protected void performApply()
	{
		applyChanges(true);
	}
}
