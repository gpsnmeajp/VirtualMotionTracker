using System;
using System.IO;
using System.Text;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;

namespace vmt_manager
{
    /// <summary>
    /// App.xaml の相互作用ロジック
    /// </summary>
    public partial class App : Application
    {
        public App()
        {
            this.DispatcherUnhandledException += App_DispatcherUnhandledException;
            TaskScheduler.UnobservedTaskException += TaskScheduler_UnobservedTaskException;
            AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;
        }

        //Thread.Sleep(100)は、入れないとnotepadが起動しないので入れている

        static bool TaskScheduler_UnobservedTaskException_Recorded = false;
        private void TaskScheduler_UnobservedTaskException(object sender, UnobservedTaskExceptionEventArgs e)
        {
            if (!TaskScheduler_UnobservedTaskException_Recorded)
            {
                TaskScheduler_UnobservedTaskException_Recorded = true;
                exceptionHandler(e.Exception, "UnobservedTaskException.log");
            }
            Thread.Sleep(100);
        }

        static bool CurrentDomain_UnhandledException_Recorded = false;
        private void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            if (!CurrentDomain_UnhandledException_Recorded)
            {
                CurrentDomain_UnhandledException_Recorded = true;
                exceptionHandler(e.ExceptionObject, "UnhandledException.log");
            }
            Thread.Sleep(100);
        }

        static bool App_DispatcherUnhandledException_Recorded = false;
        private void App_DispatcherUnhandledException(object sender, DispatcherUnhandledExceptionEventArgs e)
        {
            if (!App_DispatcherUnhandledException_Recorded)
            {
                App_DispatcherUnhandledException_Recorded = true;
                exceptionHandler(e.Exception, "DispatcherUnhandledException.log");
            }
            Thread.Sleep(100);
        }

        static void exceptionHandler(Object e, string filename)
        {
            try
            {
                Exception exception = e as Exception;
                if (exception != null)
                {
                    string msg = "VMT Manager\n";
                    msg += "=============================\n";
                    msg += "例外が発生しました。(Exception)\n";
                    msg += "=============================\n";
                    msg += exception.Message + "\n";
                    msg += "=============================\n";
                    msg += exception.StackTrace + "\n";
                    msg += "=============================\n";
                    File.WriteAllText(filename, msg, new UTF8Encoding(false));
                    System.Diagnostics.Process.Start(filename);
                }
                else
                {
                    string msg = "不明な例外が発生しました。(Unknown Exception)\n";
                    File.WriteAllText(filename, msg, new UTF8Encoding(false));
                    System.Diagnostics.Process.Start(filename);
                }
            }
            catch (Exception)
            {
                //Do noting
            }
        }
    }
}
