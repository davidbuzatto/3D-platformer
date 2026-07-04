package modelspreviewgenerator;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import javax.imageio.ImageIO;
import javax.swing.ImageIcon;
import javax.swing.JOptionPane;

/**
 *
 * @author Prof. Dr. David Buzatto
 */
public class ModelsPreviewGenerator {
    
    private static void renameFiles( File[] files ) {
        
        for ( File f : files ) {
            
            List<Character> collectedChars = new ArrayList<>();
            String name = f.getName();
            int index = 0;
            
            System.out.println( name );
            
            while ( ( index = name.indexOf( '-', index ) ) >= 0 ) {
                collectedChars.add( Character.toUpperCase( name.charAt( index + 1 ) ) );
                index++;
            }
            
            StringBuilder sb = new StringBuilder();
            int pos = 0;
            char[] chars = name.toCharArray();
            for ( int i = 0; i < chars.length; i++ ) {
                char c = chars[i];
                if ( c == '-' ) {
                    sb.append( collectedChars.get( pos++ ) );
                    i++;
                } else {
                    sb.append( c );
                }
            }
            
            System.out.println( sb.toString() );
            String filePath = f.getAbsolutePath().trim();
            System.out.println( filePath );
            String newFilePath = filePath.substring( 0, filePath.lastIndexOf( File.separator ) ) + File.separator + sb.toString();
            newFilePath = newFilePath.trim();
            System.out.println( newFilePath );
            if ( !filePath.equals( newFilePath ) ) {
                f.renameTo( new File( newFilePath ) );
            }
            
        }
        
    }
    
    public static void main( String[] args ) throws Exception {
        
        File[] files = new File( "previews" ).listFiles();
        //File[] files = new File( "../../resources/models" ).listFiles();
        
        //renameFiles( files );
        
        Arrays.sort( files, ( File a, File b ) -> {
            return a.getName().compareTo( b.getName() );
        });
        
        int totalItens = files.length;
        int columns = 15;
        int lines = totalItens / columns + ( totalItens % columns != 0 ? 1 : 0 );
        
        int hMargin = 10;
        int vMargin = 10;
        int itemW = 64;
        int itemH = 64;
        int spacing = 6;
        
        Color bgColor = new Color( 62, 62, 74 );
        
        int previewWidth = columns * itemW + ( columns - 1 ) * spacing + hMargin * 2 + 2;
        int previewHeight = lines * itemH + ( lines - 1 ) * spacing + vMargin * 2 + 2;
        
        BufferedImage previewImage = new BufferedImage( previewWidth, previewHeight, BufferedImage.TYPE_INT_ARGB );
        Graphics2D g2d = (Graphics2D) previewImage.createGraphics();
        g2d.setRenderingHint( RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON );
        
        g2d.setColor( bgColor );
        g2d.fillRoundRect( 0, 0, previewWidth - 1, previewHeight - 1, 15, 15 );
        
        for ( int i = 0; i < totalItens; i++ ) {
            
            File f = files[i];
            int row = i / columns;
            int col = i % columns;
            
            g2d.setColor( Color.WHITE );
            g2d.drawRect( hMargin + ( itemW + spacing ) * col, vMargin + ( itemH + spacing ) * row, itemW, itemH );
            g2d.drawImage( ImageIO.read( f ), hMargin + ( itemW + spacing ) * col, vMargin + ( itemH + spacing ) * row, itemW, itemH, null );
            
        }
        
        g2d.dispose();
        
        ImageIO.write( previewImage, "png", new File( "mapPieceModelAtlasPreview.png" ) );
        JOptionPane.showMessageDialog( null, new ImageIcon( previewImage ), "Preview", JOptionPane.PLAIN_MESSAGE );
        
        /*for ( File f : files ) {
            
            String baseName = f.getName().replace( ".png", "" );
            //System.out.printf( "Model %sMapPieceModel;\n", baseName );
            //System.out.printf( "_rm.%sMapPieceModel = LoadModel( \"resources/models/%s.glb\" );\n", baseName, baseName );
            //System.out.printf( "UnloadModel( _rm.%sMapPieceModel );\n", baseName );
            String enumName = "MODEL_TYPE_" + toSnakeCase( baseName ).toUpperCase();
            String modelPath = "resources/models/" + baseName + ".glb";
            //System.out.println( enumName + "," );
            
            //System.out.printf( "map[currentModel++] = (MapPieceModelTypeAndPathMapping) { .type = %s, .path = \"%s\" };\n", enumName, modelPath );
            //System.out.printf( "_rm.modelAtlas[i] = LoadModel\n", enumName, modelPath );
            System.out.printf( "_rm.modelAtlas[currentModel++] = LoadModel( \"resources/models/%s.glb\" );\n", baseName );
            
        }*/
        
    }
    
    private static String toSnakeCase( String str ) {
        
        StringBuilder sb = new StringBuilder();
        char[] chars = str.toCharArray();
        
        for ( int i = 0; i < chars.length; i++ ) {
            char c = chars[i];
            if ( Character.isUpperCase( c ) ) {
                sb.append( "_" );
            }
            sb.append( c );
        }
        
        return sb.toString().trim();
        
    }
    
}
