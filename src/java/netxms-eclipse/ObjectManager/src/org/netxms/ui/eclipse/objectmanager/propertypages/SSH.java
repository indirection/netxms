/**
 * NetXMS - open source network management system
 * Copyright (C) 2003-2021 Victor Kirhenshtein
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
package org.netxms.ui.eclipse.objectmanager.propertypages;

import java.util.List;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.ui.dialogs.PropertyPage;
import org.netxms.client.NXCObjectModificationData;
import org.netxms.client.NXCSession;
import org.netxms.client.SshKeyData;
import org.netxms.client.objects.AbstractNode;
import org.netxms.ui.eclipse.jobs.ConsoleJob;
import org.netxms.ui.eclipse.objectbrowser.widgets.ObjectSelector;
import org.netxms.ui.eclipse.objectmanager.Activator;
import org.netxms.ui.eclipse.objectmanager.Messages;
import org.netxms.ui.eclipse.shared.ConsoleSharedData;
import org.netxms.ui.eclipse.tools.WidgetHelper;
import org.netxms.ui.eclipse.widgets.LabeledText;

/**
 * "SSH" property page for node
 */
public class SSH extends PropertyPage
{
   private AbstractNode node;
   private ObjectSelector sshProxy;
   private LabeledText sshLogin;
   private LabeledText sshPassword;
   private LabeledText sshPort;
   private Combo sshKey;
   private List<SshKeyData> keyList;
   private NXCSession session;

   /* (non-Javadoc)
    * @see org.eclipse.jface.preference.PreferencePage#createContents(org.eclipse.swt.widgets.Composite)
    */
   @Override
   protected Control createContents(Composite parent)
   {
      node = (AbstractNode)getElement().getAdapter(AbstractNode.class);
      session = (NXCSession)ConsoleSharedData.getSession();

      Composite dialogArea = new Composite(parent, SWT.NONE);
      GridLayout dialogLayout = new GridLayout();
      dialogLayout.marginWidth = 0;
      dialogLayout.marginHeight = 0;
      dialogLayout.numColumns = 2;
      dialogLayout.makeColumnsEqualWidth = true;
      dialogArea.setLayout(dialogLayout);
      
      sshLogin = new LabeledText(dialogArea, SWT.NONE);
      sshLogin.setLabel("Login");
      sshLogin.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false));
      sshLogin.setText(node.getSshLogin());
      
      sshPassword = new LabeledText(dialogArea, SWT.NONE);
      sshPassword.setLabel("Password");
      sshPassword.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false));
      sshPassword.setText(node.getSshPassword());
      
      sshPort = new LabeledText(dialogArea, SWT.NONE);
      sshPort.setLabel("Port");
      sshPort.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false));
      sshPort.setText(Integer.toString(node.getSshPort()));
      
      sshKey = WidgetHelper.createLabeledCombo(dialogArea, SWT.BORDER | SWT.READ_ONLY, 
            "Key from configuration", new GridData(SWT.FILL, SWT.CENTER, true, false, 2, 1));
      
      sshProxy = new ObjectSelector(dialogArea, SWT.NONE, true);
      sshProxy.setLabel(Messages.get().Communication_Proxy);
      sshProxy.setEmptySelectionName("<default>");
      sshProxy.setObjectId(node.getSshProxyId());
      sshProxy.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 2, 1));
      syncUsersAndRefresh();
      
      return dialogArea;
   }
   
   /**
    * Get user info and refresh view
    */
   private void syncUsersAndRefresh()
   {
      ConsoleJob job = new ConsoleJob("Reloading SSH key list", null, Activator.PLUGIN_ID, null) {
         @Override
         protected void runInternal(IProgressMonitor monitor) throws Exception
         {
            keyList = session.getSshKeys(true);
            System.out.println(keyList.size());
            runInUIThread(new Runnable() {
               @Override
               public void run()
               {
                  sshKey.add("");   
                  int selection = 0;
                  for (int i = 0; i < keyList.size(); i++)
                  {
                     SshKeyData d = keyList.get(i);
                     sshKey.add(d.getName());   
                     if (d.getId() == node.getSshKeyId())
                        selection = i + 1;
                  }      
                  sshKey.select(selection);              
               }
            });
         }
         
         @Override
         protected String getErrorMessage()
         {
            return "Cannot get list of SSH key";
         }
      };
      job.setUser(false);
      job.start();  
   }


   /**
    * Apply changes
    * 
    * @param isApply true if update operation caused by "Apply" button
    */
   protected boolean applyChanges(final boolean isApply)
   {
      final NXCObjectModificationData md = new NXCObjectModificationData(node.getObjectId());
      
      if (isApply)
         setValid(false);

      try
      {
         md.setSshPort(Integer.parseInt(sshPort.getText(), 10));
      }
      catch(NumberFormatException e)
      {
         MessageDialog.openWarning(getShell(), Messages.get().Communication_Warning, "Please enter valid SSH port number");
         if (isApply)
            setValid(true);
         return false;
      }
      
      int selection = sshKey.getSelectionIndex();
      if (selection > 0)
      {
         SshKeyData d = keyList.get(selection - 1);
         md.setSshKeyId(d.getId());
      }
      else
         md.setSshKeyId(0);
         
      md.setSshProxy(sshProxy.getObjectId());
      md.setSshLogin(sshLogin.getText().trim());
      md.setSshPassword(sshPassword.getText());

      final NXCSession session = (NXCSession)ConsoleSharedData.getSession();
      new ConsoleJob(String.format("Updating SSH settings for node %s", node.getObjectName()), null, Activator.PLUGIN_ID, null) {
         @Override
         protected void runInternal(IProgressMonitor monitor) throws Exception
         {
            session.modifyObject(md);
         }

         @Override
         protected String getErrorMessage()
         {
            return String.format("Cannot update SSH settings for node %s", node.getObjectName());
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
                     SSH.this.setValid(true);
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

   /* (non-Javadoc)
    * @see org.eclipse.jface.preference.PreferencePage#performDefaults()
    */
   @Override
   protected void performDefaults()
   {
      super.performDefaults();
      sshProxy.setObjectId(0);
      sshLogin.setText("netxms");
      sshPassword.setText("");
      sshPort.setText("22");
   }
}
