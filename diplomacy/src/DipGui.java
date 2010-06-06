// DipGui.java
// All code is work of Jennifer Freeman, 2001
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
class DipGui extends JFrame
{
    private JList list;
    private JList fleet_moves;
    private JList army_moves;
    private JLabel info,me;
    private JScrollPane pane,army_pane,fleet_pane;
    private GridLayout gridlayout;
    private Box box;
    private JPanel panel;
    private String[] current_country;
    
    public DipGui()
    {
        setTitle("DipGui");
        setSize(350,300);
        addWindowListener(new WindowAdapter()
		{	public void windowClosing(WindowEvent e)
			{ System.exit(0);
			}
		} );
        current_country=Map.getValues();
        Container contentPane=this.getContentPane();




        list=new JList(current_country);
        info=new JLabel();
        army_moves=new JList();
        fleet_moves=new JList();
        MouseListener mouseListener=new MouseAdapter()
        {
          public void mouseClicked(MouseEvent e)
          { 
            Country n;
            int index = list.locationToIndex(e.getPoint());
            n=Map.getCountry(current_country[index]);
            info.setText(n.getName());
            army_moves.setListData(n.getArmyMoves().toArray());
            fleet_moves.setListData(n.getFleetMoves(Port.NoPort).toArray());
          }
        };
        list.addMouseListener(mouseListener);
        pane=new JScrollPane(list);
        pane.setPreferredSize(new Dimension(400, 100));
        fleet_pane=new JScrollPane(fleet_moves);
        army_pane=new JScrollPane(army_moves);
        panel=new JPanel();
        gridlayout=new GridLayout(1,2);
        gridlayout.setHgap(5);
        gridlayout.setVgap(5);
        panel.setLayout(gridlayout);
        panel.add(pane);
        box=Box.createVerticalBox();
        panel.add(box);
        box.add(info);
        me=new JLabel("Fleet:");
        box.add(me);
        box.add(fleet_pane);
        me=new JLabel("Army:");
        box.add(me);
        box.add(army_pane);
        contentPane.add(panel);
    }
}

