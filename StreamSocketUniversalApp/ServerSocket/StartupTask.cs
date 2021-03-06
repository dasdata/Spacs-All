﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Windows.System.Threading;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace ServerSocket
{
    public sealed class StartupTask : IBackgroundTask
    {
        private SocketServer socket;
        public void Run(IBackgroundTaskInstance taskInstance)
        {
            // 
            // TODO: Insert code to perform background work
            //
            // If you start any asynchronous methods here, prevent the task
            // from closing prematurely by using BackgroundTaskDeferral as
            // described in http://aka.ms/backgroundtaskdeferral
            //
            taskInstance.GetDeferral();
            socket = new SocketServer(9000);
            ThreadPool.RunAsync(x => {
                socket.OnError += socket_OnError;
                socket.OnDataRecived += Socket_OnDataRecived;
                socket.Star();
            });
        }

        private void Socket_OnDataRecived(string data)
        {
            socket.Send("Text Recive:" + data);
        }

        private void socket_OnError(string message)
        {
            
        }
    }
}
