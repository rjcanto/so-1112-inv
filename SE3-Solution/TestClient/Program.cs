/*
 * INSTITUTO SUPERIOR DE ENGENHARIA DE LISBOA
 * Licenciatura em Engenharia Informática e de Computadores
 *
 * Sistemas Operativos - Inverno 2011-2012
 *
 * Cliente de teste do servidor de file tracking.
 *  Modificamos o protocolo de comunicação de forma a que os argumentos para o register/unregister sejam separados por ; 
 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Net.Sockets;
using System.Net;

namespace TestClient
{
    class Program
    {
        private const int SERVERPORT = 8888;

        private static void Register(IEnumerable<string> files, string adress, ushort port)
        {
            using(TcpClient client = new TcpClient())
            {
                client.Connect(IPAddress.Loopback, SERVERPORT);

                StreamWriter output = new StreamWriter(client.GetStream());

                // Send request type line
                output.WriteLine("REGISTER");

                // Send message payload
                foreach(string file in files)
                    output.Write(string.Format(";{0}:{1}:{2}", file, adress, port));
                    //output.Write(";file1:192.0.1.155:84");
                // Send message end mark
                output.WriteLine();
                output.WriteLine("LIST_FILES");
                output.Close();
                client.Close();
            }
        }

        private static void Unregister(string file, string adress, ushort port)
        {
            using (TcpClient client = new TcpClient())
            {
                client.Connect(IPAddress.Loopback, SERVERPORT);

                StreamWriter output = new StreamWriter(client.GetStream());

                // Send request type line
                output.WriteLine("UNREGISTER");
                // Send message payload
                output.Write(string.Format(";{0}:{1}:{2}", file, adress, port));
                // Send message end mark
                output.WriteLine();

                output.Close();
                client.Close();
            }
        }

        private static void ListFiles()
        {
            using (TcpClient socket = new TcpClient())
            {
                socket.Connect(IPAddress.Loopback, SERVERPORT);

                StreamWriter output = new StreamWriter(socket.GetStream());

                // Send request type line
                output.WriteLine("LIST_FILES");
                // Send message end mark and flush it
                output.WriteLine();
                output.Flush();

                // Read response
                string line;
                StreamReader input = new StreamReader(socket.GetStream());
                while ((line = input.ReadLine()) != null && line != string.Empty)
                    Console.WriteLine(line);

                output.Close();
                socket.Close();
            }
        }

        private static void ListLocations(string fileName)
        {
            using (TcpClient socket = new TcpClient())
            {
                socket.Connect(IPAddress.Loopback, SERVERPORT);

                StreamWriter output = new StreamWriter(socket.GetStream());

                // Send request type line
                output.WriteLine("LIST_LOCATIONS");
                // Send message payload
                output.WriteLine(fileName);
                // Send message end mark and flush it
                output.Flush();

                // Read response
                string line;
                StreamReader input = new StreamReader(socket.GetStream());
                while ((line = input.ReadLine()) != null && line != string.Empty)
                    Console.WriteLine(line);

                output.Close();
                socket.Close();
            }
        }


        static void Main()
        {
			/* Several Register Commands */
            Register(new [] {"xpto", "ypto", "zpto"}, "192.1.1.1", 5555);
            Register(new[] { "xpto", "ypto" }, "192.1.1.2", 5555);
            Register(new[] { "xpto" }, "192.1.1.3", 5555);

            Console.WriteLine("List files:");
            ListFiles();
            Console.WriteLine("List locations xpto");
            ListLocations("xpto");
            Console.WriteLine("List locations ypto");
            ListLocations("ypto");
            Console.WriteLine("List locations zpto");
            ListLocations("zpto");

			Console.WriteLine("\nPress enter to continue test...");
            Console.ReadLine();
    
			/* Unregist one entry */
			Console.WriteLine("Unregist one zpto entry.");
            Unregister("zpto", "192.1.1.1", 5555);

            Console.WriteLine("List files:");
            ListFiles();
            Console.WriteLine("List locations xpto");
            ListLocations("xpto");
            Console.WriteLine("List locations ypto");
            ListLocations("ypto");
            Console.WriteLine("List locations zpto");
            ListLocations("zpto");

			Console.WriteLine("\nPress enter to continue test...");
            Console.ReadLine();

			Console.WriteLine("Unregist one xpto and ypto entries.");
            Unregister("xpto", "192.1.1.1", 5555);
            Unregister("ypto", "192.1.1.1", 5555);

            Console.WriteLine("List files:");
            ListFiles();
            Console.WriteLine("List locations xpto");
            ListLocations("xpto");
            Console.WriteLine("List locations ypto");
            ListLocations("ypto");
            Console.WriteLine("List locations zpto");
            ListLocations("zpto");
            Console.ReadLine();
        }
    }
}
