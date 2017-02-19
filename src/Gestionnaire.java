import java.util.*;
import java.io.*;
import java.net.*;
import java.lang.*;

public class Gestionnaire{
    public ServerSocket s;
    public int maxconnecte;
    public class Membre{
	public String id,IP1,IP2;
	public int Port1, Port2;

	public Membre(String d,String ip1,String ip2,int p1,int p2){
	    id=d;
	    IP1=ip1;
	    IP2=ip2;
	    Port1=p1;
	    Port2=p2;
	}

	public String toString(){
	    return "ITEM "+id+" "+IP1+" "+Port1+" "+IP2+" "+Port2+"\r\n";
	}
    }
    public ArrayList<Membre> listeM;
    public class GestService implements Runnable{
	public Socket sock;
			
	public GestService(Socket s){
	    sock=s;
	
	}
		
	public String stringToIP(String s){
	    String res="";
	    String[] array=s.split(".");
	    for(int i=0; i<array.length;i++){
		if(array[i].length()==3){
		    res+=array[i];
		}else if(array[i].length()==2){
		    res+="0"+array[i];
		}else{
		    res+="00"+array[i];
		}
		if(i!=array.length-1){
		    res+=".";
		}
	    }
	    return res;
	}

	public void run(){
	    Membre m =null;
	    try{
		BufferedReader br = new BufferedReader(new InputStreamReader(sock.getInputStream()));
		PrintWriter pw=new PrintWriter(new OutputStreamWriter(sock.getOutputStream()));
		String s = br.readLine();
		System.out.println(s);
		String[] array=s.split(" ");
		if(array[0].equals("REGI")){
		   
		    if(listeM.size()<maxconnecte){
			
			String ip1=stringToIP(array[2]),ip2=stringToIP(array[4]);
			m = new Membre(array[1],ip1,ip2,Integer.parseInt(array[3]),Integer.parseInt(array[5]));
			listeM.add(m);
			pw.print("REOK\r\n");
			pw.flush();
			sock.setSoTimeout(300000);
			while(true){
			    pw.print("RUOK\r\n");
			    pw.flush();
			    s=br.readLine();
			    if(!s.equals("IMOK\r\n")){
				    break;
			    }

			    
			}
		    }else{
			pw.println("RENO\r\n");
			pw.flush();
		    }
		}else if(array[0].equals("LIST")){
		    if(listeM.size()<10){
			pw.println("LINB 0"+listeM.size());
		    }else{
			pw.println("LINB "+listeM.size());
		    }
		    pw.flush();
		    for(int i=0;i<listeM.size();i++){
				pw.println(listeM.get(i).toString());
				pw.flush();
		    }
		}
		sock.close();
		
	    }catch(Exception e){
		listeM.remove(m);
	    }
	}
    }
    public Gestionnaire(int p,int max){
	try{
	    listeM=new ArrayList<Membre>(max);
	    s=new ServerSocket(p);
	    maxconnecte=max;
	    while(true){
	    
		Socket sock=s.accept();
		GestService gs = new GestService(sock);
		Thread t = new Thread(gs);
		t.start();
	    }
	}catch(Exception e){
	    e.printStackTrace();
	}
		
    }

    public static void main(String[] args){
	Gestionnaire g = new Gestionnaire(Integer.parseInt(args[0]),10);
    }
}
 



