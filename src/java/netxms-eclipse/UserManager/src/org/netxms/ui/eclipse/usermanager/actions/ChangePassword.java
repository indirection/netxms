/**
 * NetXMS - open source network management system
 * Copyright (C) 2003-2009 Victor Kirhenshtein
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
package org.netxms.ui.eclipse.usermanager.actions;

import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.window.Window;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.IWorkbenchWindowActionDelegate;
import org.eclipse.ui.progress.UIJob;
import org.netxms.client.NXCSession;
import org.netxms.ui.eclipse.jobs.ConsoleJob;
import org.netxms.ui.eclipse.shared.NXMCSharedData;
import org.netxms.ui.eclipse.usermanager.Activator;
import org.netxms.ui.eclipse.usermanager.dialogs.ChangePasswordDialog;

/**
 * Workbench action - change password for currently logged in user.
 *
 */
public class ChangePassword implements IWorkbenchWindowActionDelegate
{
	private IWorkbenchWindow window;
	
	/* (non-Javadoc)
	 * @see org.eclipse.ui.IWorkbenchWindowActionDelegate#dispose()
	 */
	@Override
	public void dispose()
	{
	}

	/* (non-Javadoc)
	 * @see org.eclipse.ui.IWorkbenchWindowActionDelegate#init(org.eclipse.ui.IWorkbenchWindow)
	 */
	@Override
	public void init(IWorkbenchWindow window)
	{
		this.window = window;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.ui.IActionDelegate#run(org.eclipse.jface.action.IAction)
	 */
	@Override
	public void run(IAction action)
	{
		final ChangePasswordDialog dlg = new ChangePasswordDialog(window.getShell(), true);
		if (dlg.open() == Window.OK)
		{
			ConsoleJob job = new ConsoleJob("Change password for current user", null, Activator.PLUGIN_ID, null) {
				@Override
				protected String getErrorMessage()
				{
					return "Cannot change password";
				}

				@Override
				protected void runInternal(IProgressMonitor monitor) throws Exception
				{
					NXCSession session = NXMCSharedData.getInstance().getSession();
					session.setUserPassword(session.getUserId(), dlg.getPassword(), dlg.getOldPassword());
					new UIJob("Password change notification") {
						@Override
						public IStatus runInUIThread(IProgressMonitor monitor)
						{
							MessageDialog.openInformation(window.getShell(), "Information", "Password changed successfully");
							return Status.OK_STATUS;
						}
					}.schedule();
				}
			};
			job.start();
		}
	}

	/* (non-Javadoc)
	 * @see org.eclipse.ui.IActionDelegate#selectionChanged(org.eclipse.jface.action.IAction, org.eclipse.jface.viewers.ISelection)
	 */
	@Override
	public void selectionChanged(IAction action, ISelection selection)
	{
	}
}
