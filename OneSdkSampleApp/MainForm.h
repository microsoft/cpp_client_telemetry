#pragma once

#include "NativeLogger.h"

namespace OneSdkSampleApp
{
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for MainForm
	/// </summary>
	public ref class MainForm : public System::Windows::Forms::Form
	{
	public:
		MainForm(void)
		{
			InitializeComponent();

			m_pLogger = new NativeLogger();
		}
	private: System::Windows::Forms::CheckBox^  checkBoxUseUtc;
	public:

	protected:

		NativeLogger * m_pLogger = nullptr;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MainForm()
		{
			if (components)
			{
				delete components;
			}

			//if (pLogger != nullptr)
			//{
			//	delete pLogger;
			//}
		}

	private: System::Windows::Forms::Button^  buttonStart;
	private: System::Windows::Forms::Button^  buttonStop;
	private: System::Windows::Forms::Button^  buttonPause;
	private: System::Windows::Forms::Button^  buttonResume;
	private: System::Windows::Forms::Button^  buttonStartPeriodicSender;
	private: System::Windows::Forms::Button^  buttonStopPeriodicSender;
	private: System::Windows::Forms::Button^  buttonSendEvent1;
	private: System::Windows::Forms::Button^  buttonSendEvent5;
	private: System::Windows::Forms::Button^  buttonSendEvent50;
	private: System::Windows::Forms::Button^  buttonSendEvent200;
	private: System::Windows::Forms::Button^  buttonRunUnitTests;
	private: System::Windows::Forms::CheckBox^  checkBoxMark;
	private: System::Windows::Forms::CheckBox^  checkBoxHash;
	private: System::Windows::Forms::CheckBox^  checkBoxDrop;
	private: System::Windows::Forms::CheckBox^  checkBoxRealTime;
	private: System::Windows::Forms::CheckBox^  checkBoxCritical;
	private: System::Windows::Forms::CheckBox^  checkBoxInvalid;
	private: System::Windows::Forms::CheckBox^  checkBoxWithAuth;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->buttonStart = (gcnew System::Windows::Forms::Button());
			this->buttonStop = (gcnew System::Windows::Forms::Button());
			this->buttonPause = (gcnew System::Windows::Forms::Button());
			this->buttonResume = (gcnew System::Windows::Forms::Button());
			this->buttonStartPeriodicSender = (gcnew System::Windows::Forms::Button());
			this->buttonStopPeriodicSender = (gcnew System::Windows::Forms::Button());
			this->buttonSendEvent1 = (gcnew System::Windows::Forms::Button());
			this->buttonSendEvent5 = (gcnew System::Windows::Forms::Button());
			this->buttonSendEvent50 = (gcnew System::Windows::Forms::Button());
			this->buttonSendEvent200 = (gcnew System::Windows::Forms::Button());
			this->buttonRunUnitTests = (gcnew System::Windows::Forms::Button());
			this->checkBoxMark = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxHash = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxDrop = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxRealTime = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxCritical = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxInvalid = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxWithAuth = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxUseUtc = (gcnew System::Windows::Forms::CheckBox());
			this->SuspendLayout();
			// 
			// buttonStart
			// 
			this->buttonStart->Location = System::Drawing::Point(13, 13);
			this->buttonStart->Name = L"buttonStart";
			this->buttonStart->Size = System::Drawing::Size(75, 23);
			this->buttonStart->TabIndex = 0;
			this->buttonStart->Text = L"&Start";
			this->buttonStart->UseVisualStyleBackColor = true;
			this->buttonStart->Click += gcnew System::EventHandler(this, &MainForm::buttonStart_Click);
			// 
			// buttonStop
			// 
			this->buttonStop->Location = System::Drawing::Point(95, 12);
			this->buttonStop->Name = L"buttonStop";
			this->buttonStop->Size = System::Drawing::Size(75, 23);
			this->buttonStop->TabIndex = 1;
			this->buttonStop->Text = L"S&top";
			this->buttonStop->UseVisualStyleBackColor = true;
			this->buttonStop->Click += gcnew System::EventHandler(this, &MainForm::buttonStop_Click);
			// 
			// buttonPause
			// 
			this->buttonPause->Location = System::Drawing::Point(177, 12);
			this->buttonPause->Name = L"buttonPause";
			this->buttonPause->Size = System::Drawing::Size(75, 23);
			this->buttonPause->TabIndex = 2;
			this->buttonPause->Text = L"&Pause";
			this->buttonPause->UseVisualStyleBackColor = true;
			// 
			// buttonResume
			// 
			this->buttonResume->Location = System::Drawing::Point(259, 12);
			this->buttonResume->Name = L"buttonResume";
			this->buttonResume->Size = System::Drawing::Size(75, 23);
			this->buttonResume->TabIndex = 3;
			this->buttonResume->Text = L"&Resume";
			this->buttonResume->UseVisualStyleBackColor = true;
			// 
			// buttonStartPeriodicSender
			// 
			this->buttonStartPeriodicSender->Location = System::Drawing::Point(95, 59);
			this->buttonStartPeriodicSender->Name = L"buttonStartPeriodicSender";
			this->buttonStartPeriodicSender->Size = System::Drawing::Size(156, 23);
			this->buttonStartPeriodicSender->TabIndex = 4;
			this->buttonStartPeriodicSender->Text = L"Start Periodic Sender";
			this->buttonStartPeriodicSender->UseVisualStyleBackColor = true;
			// 
			// buttonStopPeriodicSender
			// 
			this->buttonStopPeriodicSender->Location = System::Drawing::Point(95, 89);
			this->buttonStopPeriodicSender->Name = L"buttonStopPeriodicSender";
			this->buttonStopPeriodicSender->Size = System::Drawing::Size(156, 23);
			this->buttonStopPeriodicSender->TabIndex = 5;
			this->buttonStopPeriodicSender->Text = L"Stop Periodic Sender";
			this->buttonStopPeriodicSender->UseVisualStyleBackColor = true;
			// 
			// buttonSendEvent1
			// 
			this->buttonSendEvent1->Location = System::Drawing::Point(95, 119);
			this->buttonSendEvent1->Name = L"buttonSendEvent1";
			this->buttonSendEvent1->Size = System::Drawing::Size(156, 23);
			this->buttonSendEvent1->TabIndex = 6;
			this->buttonSendEvent1->Text = L"Send 1 event";
			this->buttonSendEvent1->UseVisualStyleBackColor = true;
			this->buttonSendEvent1->Click += gcnew System::EventHandler(this, &MainForm::buttonSendEvent1_Click);
			// 
			// buttonSendEvent5
			// 
			this->buttonSendEvent5->Location = System::Drawing::Point(96, 148);
			this->buttonSendEvent5->Name = L"buttonSendEvent5";
			this->buttonSendEvent5->Size = System::Drawing::Size(155, 23);
			this->buttonSendEvent5->TabIndex = 7;
			this->buttonSendEvent5->Text = L"Send 5 events";
			this->buttonSendEvent5->UseVisualStyleBackColor = true;
			this->buttonSendEvent5->Click += gcnew System::EventHandler(this, &MainForm::buttonSendEvent5_Click);
			// 
			// buttonSendEvent50
			// 
			this->buttonSendEvent50->Location = System::Drawing::Point(95, 177);
			this->buttonSendEvent50->Name = L"buttonSendEvent50";
			this->buttonSendEvent50->Size = System::Drawing::Size(155, 21);
			this->buttonSendEvent50->TabIndex = 8;
			this->buttonSendEvent50->Text = L"Send 50 events";
			this->buttonSendEvent50->UseVisualStyleBackColor = true;
			this->buttonSendEvent50->Click += gcnew System::EventHandler(this, &MainForm::buttonSendEvent50_Click);
			// 
			// buttonSendEvent200
			// 
			this->buttonSendEvent200->Location = System::Drawing::Point(96, 205);
			this->buttonSendEvent200->Name = L"buttonSendEvent200";
			this->buttonSendEvent200->Size = System::Drawing::Size(154, 23);
			this->buttonSendEvent200->TabIndex = 9;
			this->buttonSendEvent200->Text = L"Send 200 events";
			this->buttonSendEvent200->UseVisualStyleBackColor = true;
			this->buttonSendEvent200->Click += gcnew System::EventHandler(this, &MainForm::buttonSendEvent200_Click);
			// 
			// buttonRunUnitTests
			// 
			this->buttonRunUnitTests->Location = System::Drawing::Point(95, 234);
			this->buttonRunUnitTests->Name = L"buttonRunUnitTests";
			this->buttonRunUnitTests->Size = System::Drawing::Size(154, 23);
			this->buttonRunUnitTests->TabIndex = 10;
			this->buttonRunUnitTests->Text = L"Run Unit Tests";
			this->buttonRunUnitTests->UseVisualStyleBackColor = true;
			// 
			// checkBoxMark
			// 
			this->checkBoxMark->AutoSize = true;
			this->checkBoxMark->Location = System::Drawing::Point(370, 17);
			this->checkBoxMark->Name = L"checkBoxMark";
			this->checkBoxMark->Size = System::Drawing::Size(50, 17);
			this->checkBoxMark->TabIndex = 11;
			this->checkBoxMark->Text = L"Mark";
			this->checkBoxMark->UseVisualStyleBackColor = true;
			// 
			// checkBoxHash
			// 
			this->checkBoxHash->AutoSize = true;
			this->checkBoxHash->Location = System::Drawing::Point(370, 41);
			this->checkBoxHash->Name = L"checkBoxHash";
			this->checkBoxHash->Size = System::Drawing::Size(51, 17);
			this->checkBoxHash->TabIndex = 12;
			this->checkBoxHash->Text = L"Hash";
			this->checkBoxHash->UseVisualStyleBackColor = true;
			// 
			// checkBoxDrop
			// 
			this->checkBoxDrop->AutoSize = true;
			this->checkBoxDrop->Location = System::Drawing::Point(370, 65);
			this->checkBoxDrop->Name = L"checkBoxDrop";
			this->checkBoxDrop->Size = System::Drawing::Size(49, 17);
			this->checkBoxDrop->TabIndex = 13;
			this->checkBoxDrop->Text = L"Drop";
			this->checkBoxDrop->UseVisualStyleBackColor = true;
			// 
			// checkBoxRealTime
			// 
			this->checkBoxRealTime->AutoSize = true;
			this->checkBoxRealTime->Location = System::Drawing::Point(370, 89);
			this->checkBoxRealTime->Name = L"checkBoxRealTime";
			this->checkBoxRealTime->Size = System::Drawing::Size(71, 17);
			this->checkBoxRealTime->TabIndex = 14;
			this->checkBoxRealTime->Text = L"RealTime";
			this->checkBoxRealTime->UseVisualStyleBackColor = true;
			// 
			// checkBoxCritical
			// 
			this->checkBoxCritical->AutoSize = true;
			this->checkBoxCritical->Location = System::Drawing::Point(370, 114);
			this->checkBoxCritical->Name = L"checkBoxCritical";
			this->checkBoxCritical->Size = System::Drawing::Size(57, 17);
			this->checkBoxCritical->TabIndex = 15;
			this->checkBoxCritical->Text = L"Critical";
			this->checkBoxCritical->UseVisualStyleBackColor = true;
			// 
			// checkBoxInvalid
			// 
			this->checkBoxInvalid->AutoSize = true;
			this->checkBoxInvalid->Location = System::Drawing::Point(370, 138);
			this->checkBoxInvalid->Name = L"checkBoxInvalid";
			this->checkBoxInvalid->Size = System::Drawing::Size(57, 17);
			this->checkBoxInvalid->TabIndex = 16;
			this->checkBoxInvalid->Text = L"Invalid";
			this->checkBoxInvalid->UseVisualStyleBackColor = true;
			// 
			// checkBoxWithAuth
			// 
			this->checkBoxWithAuth->AutoSize = true;
			this->checkBoxWithAuth->Location = System::Drawing::Point(370, 162);
			this->checkBoxWithAuth->Name = L"checkBoxWithAuth";
			this->checkBoxWithAuth->Size = System::Drawing::Size(73, 17);
			this->checkBoxWithAuth->TabIndex = 17;
			this->checkBoxWithAuth->Text = L"With Auth";
			this->checkBoxWithAuth->UseVisualStyleBackColor = true;
			// 
			// checkBoxUseUtc
			// 
			this->checkBoxUseUtc->AutoSize = true;
			this->checkBoxUseUtc->Location = System::Drawing::Point(370, 186);
			this->checkBoxUseUtc->Name = L"checkBoxUseUtc";
			this->checkBoxUseUtc->Size = System::Drawing::Size(70, 17);
			this->checkBoxUseUtc->TabIndex = 18;
			this->checkBoxUseUtc->Text = L"Use UTC";
			this->checkBoxUseUtc->UseVisualStyleBackColor = true;
			// 
			// MainForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(459, 265);
			this->Controls->Add(this->checkBoxUseUtc);
			this->Controls->Add(this->checkBoxWithAuth);
			this->Controls->Add(this->checkBoxInvalid);
			this->Controls->Add(this->checkBoxCritical);
			this->Controls->Add(this->checkBoxRealTime);
			this->Controls->Add(this->checkBoxDrop);
			this->Controls->Add(this->checkBoxHash);
			this->Controls->Add(this->checkBoxMark);
			this->Controls->Add(this->buttonRunUnitTests);
			this->Controls->Add(this->buttonSendEvent200);
			this->Controls->Add(this->buttonSendEvent50);
			this->Controls->Add(this->buttonSendEvent5);
			this->Controls->Add(this->buttonSendEvent1);
			this->Controls->Add(this->buttonStopPeriodicSender);
			this->Controls->Add(this->buttonStartPeriodicSender);
			this->Controls->Add(this->buttonResume);
			this->Controls->Add(this->buttonPause);
			this->Controls->Add(this->buttonStop);
			this->Controls->Add(this->buttonStart);
			this->Name = L"MainForm";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"One SDK Sample App";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	
private: 

		System::Void buttonStart_Click(System::Object^  sender, System::EventArgs^  e)
		{
			bool useUtc = this->checkBoxUseUtc->Checked;
			m_pLogger->Start(useUtc);
		}

		System::Void buttonStop_Click(System::Object^  sender, System::EventArgs^  e)
		{
			m_pLogger->Stop();
		}
		
		System::Void buttonSendEvent1_Click(System::Object^  sender, System::EventArgs^  e)
		{
            SendEvents(1);
		}
	
		System::Void buttonSendEvent5_Click(System::Object^  sender, System::EventArgs^  e)
		{
            SendEvents(5);
		}

		System::Void buttonSendEvent50_Click(System::Object^  sender, System::EventArgs^  e)
		{
            SendEvents(50);
		}

		System::Void buttonSendEvent200_Click(System::Object^  sender, System::EventArgs^  e)
		{
            SendEvents(200);
		}

        System::Void SendEvents(int count)
        {
            bool isCritical = this->checkBoxCritical->Checked;
            bool isRealtime = this->checkBoxRealTime->Checked;

            m_pLogger->LogEvents(count, isCritical, isRealtime);
        }
};
}
