﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace D2BSLoader
{
	public partial class Options : Form
	{
		public Options(string path, string exe, string args)
		{
			InitializeComponent();

			Path.Text = path;
			Exe.Text = exe;
			Args.Text = args;
		}

		private void OK_Click(object sender, EventArgs e)
		{
			if(File.Exists(Path.Text + System.IO.Path.DirectorySeparatorChar + Exe.Text))
			{
				Main.SaveSettings(Path.Text, Exe.Text, Args.Text);
				Close();
			}
			else
				MessageBox.Show("Invalid path and/or executable!", "D2BS");
		}

		private void Cancel_Click(object sender, EventArgs e)
		{
			Close();
		}

		private void FindPath_Click(object sender, EventArgs e)
		{
			FolderBrowserDialog fbd = new FolderBrowserDialog();
			fbd.ShowNewFolderButton = false;
			fbd.RootFolder = Environment.SpecialFolder.MyComputer;
			if(DialogResult.OK == fbd.ShowDialog())
				Path.Text = fbd.SelectedPath;
		}

		private void FindExe_Click(object sender, EventArgs e)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filter = "Executable Files|*.exe";
			ofd.Multiselect = false;
			ofd.AutoUpgradeEnabled = true;
			ofd.InitialDirectory = Path.Text;
			if(DialogResult.OK == ofd.ShowDialog())
				Exe.Text = ofd.SafeFileName;
		}
	}
}
