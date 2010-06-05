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
    private JScrollPane bar,army_bar,fleet_bar;
    private GridLayout gah;
    private Box moo;
    private JPanel moo2;
    private String[] m;
    public DipGui()
    {
        setTitle("DipGui");
        setSize(350,300);
        addWindowListener(new WindowAdapter()
		{	public void windowClosing(WindowEvent e)
			{ System.exit(0);
			}
		} );
        m=Map.getValues();
        Container contentPane=this.getContentPane();




        list=new JList(m);
        info=new JLabel();
        army_moves=new JList();
        fleet_moves=new JList();
        MouseListener mouseListener=new MouseAdapter()
        {
          public void mouseClicked(MouseEvent e)
          { 
            Country n;
            int index = list.locationToIndex(e.getPoint());
            n=Map.getCountry((String) m[index]);
            info.setText(n.getName());
            army_moves.setListData(n.getArmyMoves().toArray());
            fleet_moves.setListData(n.getFleetMoves(Port.NoPort).toArray());
          }
        };
        list.addMouseListener(mouseListener);
        bar=new JScrollPane(list);
        bar.setPreferredSize(new Dimension(400, 100));
        fleet_bar=new JScrollPane(fleet_moves);
        army_bar=new JScrollPane(army_moves);
        moo2=new JPanel();
        gah=new GridLayout(1,2);
        gah.setHgap(5);
        gah.setVgap(5);
        moo2.setLayout(gah);
        moo2.add(bar);
        moo=Box.createVerticalBox();
        moo2.add(moo);
        moo.add(info);
        me=new JLabel("Fleet:");
        moo.add(me);
        moo.add(fleet_bar);
        me=new JLabel("Army:");
        moo.add(me);
        moo.add(army_bar);
        contentPane.add(moo2);
    }
}

