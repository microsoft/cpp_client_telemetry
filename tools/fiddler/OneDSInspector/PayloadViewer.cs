//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace OneDSInspector
{
    public partial class PayloadViewer : UserControl
    {
        /// <summary> 
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        private CheckBox m_compactJsonCheckBox;
        private RichTextBox m_mainText;

        public event EventHandler ViewerConfigurationChanged;

        /// <summary> 
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        public PayloadViewer()
        {
            InitializeComponent();
        }

        public void SetText(List<string> textList)
        {
            // determine the text width and set our width accordingly
            // this allows to extend control beyond the default ~3510 characters

            int maxWidth = 0;

            for (int i = 0; i < textList.Count; i++)
            {
                int curWidth = TextRenderer.MeasureText(textList[i], this.m_mainText.Font).Width;
                maxWidth = (maxWidth < curWidth ? curWidth : maxWidth);
            }

            // adding marging here is a magic workaround - adding 7 to MarginRight ensures that the text does not wrap
            this.m_mainText.RightMargin = maxWidth + this.m_mainText.Margin.Left + this.m_mainText.Margin.Right + 1;

            this.m_mainText.Lines = textList.ToArray();
        }

        public bool IsCompactJsonOutputRequested()
        {
            return m_compactJsonCheckBox.Checked;
        }

        private void PayloadViewer_Layout(object sender, LayoutEventArgs e)
        {
            m_mainText.Width = Math.Max(0, this.Width - 2 * m_mainText.Left);
            m_mainText.Height = Math.Max(0, this.Height - 2 * m_mainText.Top);
        }

        private void InitializeComponent()
        {
            this.m_compactJsonCheckBox = new System.Windows.Forms.CheckBox();
            this.m_mainText = new System.Windows.Forms.RichTextBox();
            this.SuspendLayout();
            // 
            // m_compactJsonCheckBox
            // 
            this.m_compactJsonCheckBox.AutoSize = true;
            this.m_compactJsonCheckBox.Location = new System.Drawing.Point(0, 0);
            this.m_compactJsonCheckBox.Name = "m_compactJsonCheckBox";
            this.m_compactJsonCheckBox.Size = new System.Drawing.Size(174, 17);
            this.m_compactJsonCheckBox.TabIndex = 0;
            this.m_compactJsonCheckBox.Text = "Produce compact JSON output";
            this.m_compactJsonCheckBox.UseVisualStyleBackColor = true;
            this.m_compactJsonCheckBox.CheckedChanged += new System.EventHandler(this.m_compactJsonCheckBox_CheckedChanged);
            // 
            // m_mainText
            // 
            this.m_mainText.BackColor = System.Drawing.SystemColors.Window;
            this.m_mainText.Font = new System.Drawing.Font("Courier New", 8.25F);
            this.m_mainText.Location = new System.Drawing.Point(0, 25);
            this.m_mainText.Name = "m_mainText";
            this.m_mainText.ReadOnly = true;
            this.m_mainText.Size = new System.Drawing.Size(300, 200);
            this.m_mainText.TabIndex = 0;
            this.m_mainText.Text = "";
            this.m_mainText.WordWrap = false;
            // 
            // PayloadViewer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.m_compactJsonCheckBox);
            this.Controls.Add(this.m_mainText);
            this.Name = "PayloadViewer";
            this.Size = new System.Drawing.Size(303, 203);
            this.Layout += new System.Windows.Forms.LayoutEventHandler(this.PayloadViewer_Layout);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        private void m_compactJsonCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            this.ViewerConfigurationChanged?.Invoke(this, e);
        }
    }
}
