using UnityEngine;
using UnityEngine.UI;
using System.Collections;

using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.IO;
using System.Runtime.InteropServices;

public class UDPManager : MonoBehaviour
{

    //receiveThread is the thread that will do the receiving of the data over the network.
    Thread receiveThread;

    //client is the actual client that does the receiving of data over the network.
    UdpClient client;

    //port is the port at which the program will receive packets.
    public int port; 

    //lastReceivedUDPPacket shows the last received packet. Can be used for debug.
    public string lastReceivedUDPPacket = "";

    //tagdata is the class in which all the data that is received over UDP is saved. This class is defined in the file TagData in the scripts folder.
    public TagData tagdata;

    //struct_tagdata was used before when trying to send the entire struct over the network. Unused right now.
    public struct struct_tagdata
    {
        public double xGlass, yGlass, zGlass, IdGlass;
        public double xTag, yTag, zTag, IdTag;
        public double xAnc0, yAnc0, zAnc0;
    }

    public void Start()
    {
        init();
        tagdata = new TagData();
    }

    //ONGUI is used for printing the Debug Data in the top left corner of the screen.
    void OnGUI()
    {
        Rect rectObj = new Rect(40, 10, 200, 400);
        GUIStyle style = new GUIStyle();
        style.alignment = TextAnchor.UpperLeft;
        style.normal.textColor = Color.white;
        GUI.Box(rectObj, "Reading port: " + port + "\nLast Packet: \n" + lastReceivedUDPPacket, style);
    }

    //initialisation
    private void init()
    {
        //This defines the port
        port = 45454;

        //Creates new thread to do the receiving of data in.
        receiveThread = new Thread(
            new ThreadStart(ReceiveData));
        receiveThread.IsBackground = true;
        receiveThread.Start();

    }

    //Receive thread
    private void ReceiveData()
    {

        client = new UdpClient(port);
        while (true)
        {

            try
            {
                int method = 1; // Determines the method used. Currently only method 1 works.

                IPEndPoint anyIP = new IPEndPoint(IPAddress.Any, 0);

                byte[] data = client.Receive(ref anyIP);
                
                if (method == 1) //This method works. It is not as nice as sending the entire struct, but it works. All info is sent through a string which will have to be split up here.
                {
                    string text = Encoding.UTF8.GetString(data);

                    lastReceivedUDPPacket = text;

                    string[] values = text.Split(',');               
                    
                    tagdata.xGlass = Convert.ToDouble(values[0]);
                    tagdata.yGlass = Convert.ToDouble(values[1]);
                    tagdata.zGlass = Convert.ToDouble(values[2]);
                    tagdata.IdGlass = Convert.ToDouble(values[3]);
                    tagdata.xTag = Convert.ToDouble(values[4]);
                    tagdata.yTag = Convert.ToDouble(values[5]);
                    tagdata.zTag = Convert.ToDouble(values[6]);
                    tagdata.IdTag = Convert.ToDouble(values[7]);
                    tagdata.xAnc0 = Convert.ToDouble(values[8]);
                    tagdata.yAnc0 = Convert.ToDouble(values[9]);
                    tagdata.zAnc0 = Convert.ToDouble(values[10]);

                    lastReceivedUDPPacket = tagdata.xGlass.ToString() + "\n" + tagdata.yGlass.ToString() + "\n" + tagdata.zGlass.ToString() + "\n" + tagdata.IdGlass.ToString() + "\n" + tagdata.xTag.ToString() + "\n" + tagdata.yTag.ToString() + "\n" + tagdata.zTag.ToString() + "\n" + tagdata.IdTag.ToString() + "\n" + tagdata.xAnc0.ToString() + "\n" + tagdata.yAnc0.ToString() + "\n" + tagdata.zAnc0.ToString();
                }
                else if (method == 2) //This method does not work. Earlier attempts have been made to send the entire struct with tagdata's over the network. In the end this did not work properly.
                {
                    struct_tagdata tagdatastruct;
                    MemoryStream mystream = new MemoryStream(data);

                    tagdatastruct = Deserialize(mystream);

                    lastReceivedUDPPacket = tagdatastruct.xGlass.ToString() + "test";
                }
            }
            catch (Exception)
            {

            }
        }
    }

    //This public function will return the last packet that was received.
    public string getLatestUDPPacket()
    {
        return lastReceivedUDPPacket;
    }

    public struct_tagdata Deserialize(Stream stream) //This function was used for the failed attempt of sending the entire struct over the network.
    {
        BinaryReader br = new BinaryReader(stream);
        struct_tagdata data = new struct_tagdata();

        data.xGlass = br.ReadDouble();
        data.yGlass = br.ReadDouble();
        data.zGlass = br.ReadDouble();
        data.IdGlass = br.ReadDouble();
        data.xTag = br.ReadDouble();
        data.yTag = br.ReadDouble();
        data.zTag = br.ReadDouble();
        data.IdTag = br.ReadDouble();
        data.xAnc0 = br.ReadDouble();
        data.yAnc0 = br.ReadDouble();
        data.zAnc0 = br.ReadDouble();

        return data;
    }

    struct_tagdata ByteArrayToNewStuff(byte[] bytes) //This function was also used for the failed attempt of sending the entire struct over the network.
    {
        GCHandle handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
        struct_tagdata stuff = (struct_tagdata)Marshal.PtrToStructure(
            handle.AddrOfPinnedObject(), typeof(struct_tagdata));
        handle.Free();
        return stuff;
    }
}