using System;
using System.IO;
using System.Xml.Linq;

namespace AutoSubmit
{
    /// <summary>
    /// This is a basic script that shows a submission to Codesign.Net.
    /// In addition it also invokes the job watcher that will continue 
    /// to monitor the job until completion
    /// 
    /// REQUIREMENTS: The user needs to add the appropriate references to this project:
    /// (1) CODESIGN.PolictyManager.dll
    /// (2) CODESIGN.Submitter.dll
    /// (3) MS.IT.BGCOE.RDS.Common.MultiThreadedFileCopier.dll
    /// (4) MS.IT.BGCOE.RDS.LongFile.IO.FileIOEx.dll
    /// These are located in "%ProgramFiles%\CODESIGN\CodeSign.Submitter" (32 bit) or "%ProgramFiles(x86)%\CODESIGN\CodeSign.Submitter" (64 bit)
    /// </summary>
    class Sample
    {
        static void Main(string[] args)
        {
            string file = args[0];  // XML argument

            // If more than one parameter supplied, assume that the second param is dir
            if (args.Length>1)
            {
                // If dir, then change dir to 3rd arg
                if (args[1].Equals("-dir"))
                {
                    Console.WriteLine("cd " + args[2]);
                    Directory.SetCurrentDirectory(args[2]);
                }
            }
            CodesignSubmission sample = new CodesignSubmission();
            sample.CreateSubmission(file);
        }
    }

    class CodesignSubmission
    {
        public void CreateSubmission(string xmlFile)
        {
            CODESIGN.Submitter.Job job = null;
            try
            {
                //Initialize the Codesign.Submitter object
                //"Codesign" represents the server - constant value do not change
                //9556 represents the port - constant value do not change
                job = CODESIGN.Submitter.Job.Initialize("codesignshimlbcp1.redmond.corp.microsoft.com", 9556, true);

                // Sets the Partial return flag option.
                // False - If any files fail signing you will not get any files back.
                // True - Only retrieve successfully signed files.
                job.IsAllowReturnPartial = true;        // default is false

                //Read in the XML document
                XDocument doc = XDocument.Load(xmlFile);

                // This is reference information that can be displayed or used in searches
                job.Description = doc.Root.Element("description").Value;
                job.Keywords = doc.Root.Element("keywords").Value;

                // This call selects a certificate from the ones allowed for this user
                // You may pick only one Authenticode certificate
                // You may pick only one Strong Name certificate
                // You may pick an Authenticode and Strong Name certificate at the same time
                // Example:
                job.SelectCertificate(doc.Root.Element("cert").Value); // You must have permissions to the requested cert

                // These calls add notification subscriptions to the job. A number of others are 
                // available, these are the standard ones.
                // Check the CODESIGN.NotificationEventTypeEnum enumaration for a complete list.
                //job.SetNotification(job.Submitter, new CODESIGN.NotificationEventTypeEnum[] {CODESIGN.NotificationEventTypeEnum.JobCompletionFailure,CODESIGN.NotificationEventTypeEnum.JobCompletionSuccess,CODESIGN.NotificationEventTypeEnum.JobVirusScanFailure});

                var files = doc.Root.Element("files").Descendants();

                foreach (XElement file in files)
                {
                    // This call adds an entire directory tree to the job, duplicating its structure in
                    // the submission share and making all metadata the same for each file.
                    string path = file.Attribute("path").Value;
                    if ((!File.Exists(path))&&(!Directory.Exists(path)))
                    {
                        if (path[0] != '\\') {
                            path = '\\' + path;
                        }
                        path = Directory.GetCurrentDirectory() + path;
                    }
                    // Try to check if path exists again
                    if ((!File.Exists(path)) && (!Directory.Exists(path)))
                        {
                        throw new Exception("Invalid path supplied: " + path);
                    }
                    job.AddFile(path, file.Attribute("displayName").Value, file.Attribute("displayUrl").Value, CODESIGN.JavaPermissionsTypeEnum.None);
                }

                // This will populate the approvers to the job
                // NOTE: these users are invalid, must substitute real user aliases who are authorized for approval
                // You need at minimum two approvers - you may add as many as necessary
                // Approvers must be entered in the system to approve jobs
                // Approvers CANNOT be the job submitter
                job.AddApprover("maxgolov");
                job.AddApprover("abpanwar");

                // This call sends the job to the back end for processing
                job.Send();

                // This call displays the job number, assigned during the send process
                Console.WriteLine("Job Number is: {0}", job.JobNumber);
                Console.WriteLine("Job Completion Path is: {0}", job.JobCompletionPath);
            }
            catch (Exception exc)
            {
                Console.WriteLine("Job submission failed: {0}", CODESIGN.EventLogProxy.GetMessage(exc));
                foreach (CODESIGN.Submitter.JobError je in job.ErrorList.Values)
                {
                    Console.WriteLine(je.Number + ":" + je.Description + " {" + je.Explanation + "}");
                }

            }
            Console.WriteLine("Press any key to continue");
            Console.Read();
        }
    }
}
