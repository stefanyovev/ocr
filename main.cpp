
   #define Title "OCR for Bruce 6 \n" \
                   " freelancer.com/u/stefanyovev.html\n\n"

   #define err1 " ERROR: Cannot find my internals."
   #define err2 " ERROR: Cannot open file."
   #define err3 " ERROR: Invalid file format. BMP only."
   #define err4 " ERROR: Cannot allocate memory."
   #define err5 " ERROR: Cannot write file."
   #define err0 " Done. "
   #define die(n) {printf("%s\n",err##n);system("pause");exit(n);}
   #define WIN32_LEAN_AND_MEAN
   #include <windows.h>
   #include <iostream>
   #include <cmath>

   struct bmp {
      BYTE *d;
      DWORD w, h;
      void load( char *fname ){
         FILE *f = fopen( fname, "rb" );
         if( !f ) die(2);
         BITMAPFILEHEADER h1; BITMAPINFOHEADER h2;
         fread( &h1, sizeof(h1), 1, f ); fread( &h2, sizeof(h2), 1, f );
         if( h2.biWidth < 0 || h2.biWidth > 10000 || h2.biHeight < 0 ) die(3);
         w = h2.biWidth; h = h2.biHeight; BYTE pad = 4-(w*3)%4; pad = pad==4 ? 0:pad;
         if( d ) delete d; d = new BYTE [w*h*3+pad*h];
         if( !d ) die(4);
         fseek( f, h1.bfOffBits, SEEK_SET ); for( int i=0; i<h; i++ ) fread( d+i*w*3, w*3+pad, 1, f ); fclose( f );
         for( int i=0; i<w*h*3; i+=3 ) d[i]=d[i+1]=d[i+2] = ((d[i]+d[i+1]+d[i+2]<500)?255:0);
      }
      bmp() : d(0), w(0), h(0) {}
      bmp( char *fname ) : d(0), w(0), h(0) { load( fname ); }
      inline DWORD get( int x, int y ){
         if( x<0 || x>w-1 || y<0 || y>h-1 ) return 0;
         BYTE *p = d+((h-y-1)*w+x)*3;
         return *p << 16 | *(p+1) << 8 | *(p+2);
      };
      inline void set( DWORD x, DWORD y, DWORD v ){
         BYTE *p = d+((h-y-1)*w+x)*3;
         *p = (v&0xFF0000)>>16;
         *(p+1) = (v&0xFF00)>>8;
         *(p+2) = v&0xFF;
      };
      void save( char *fname ){
         int pad=0;
         while( (w+pad)%4 != 0 ) pad++;
         BITMAPFILEHEADER h1; BITMAPINFOHEADER h2;
         memset( &h1, 0, sizeof(BITMAPFILEHEADER) );
         memset( &h2, 0, sizeof(BITMAPINFOHEADER) );
      	h1.bfType = 0x4d42;
      	h1.bfReserved1 = 0;
      	h1.bfReserved2 = 0;
      	h1.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + w*h*3 + (h*pad);
      	h1.bfOffBits = 0x36;
      	h2.biSize = sizeof(BITMAPINFOHEADER);
      	h2.biWidth = w;
      	h2.biHeight = h;
      	h2.biPlanes = 1;
      	h2.biBitCount = 24;
      	h2.biCompression = BI_RGB;	
      	h2.biSizeImage = 0;
      	h2.biXPelsPerMeter = 0x0ec4;
      	h2.biYPelsPerMeter = 0x0ec4;
      	h2.biClrUsed = 0;
      	h2.biClrImportant = 0;
      	FILE *f = fopen( fname, "wb" );
      	fwrite( &h1, 1, sizeof(h1), f );
      	fwrite( &h2, 1, sizeof(h2), f );
      	for( int i=0; i<h; i++ ){
            fwrite( d+i*w*3, 1, w*3, f );
            fwrite( d, 1, pad, f );
         }
      	fclose( f );
      }
   }; 

   struct block {
      DWORD id;
      bmp* src;
      DWORD x1,y1,x2,y2,w,h,m;
      BYTE fn;
      char v [3];
      void suck( DWORD x, DWORD y ){
         #define ndwords 25*1024*1024 // 100MB static
         static DWORD nextid=1; id=nextid++; if( nextid==0xFFFFFF ) nextid=1;
         static DWORD M [ndwords]; DWORD Mi=0;
         x1=x2=x; y1=y2=y; w=h=m=fn=0; v[0]=v[1]=v[2]=0;
         M[Mi++] = x; M[Mi++] = y;
         while( Mi > 0 ){
            y = M[--Mi]; x = M[--Mi];
            if( src->get(x,y) != id ) { src->set( x, y, id ); m++; }
            if( x < x1 ) x1 = x;
            if( x > x2 ) x2 = x;
            if( y < y1 ) y1 = y;
            if( y > y2 ) y2 = y;
            if( Mi > ndwords-8 ) break;
            if( src->get(x+1,y) == 0xFFFFFF ) { M[Mi++] = x+1; M[Mi++] = y; }
            if( src->get(x,y+1) == 0xFFFFFF ) { M[Mi++] = x; M[Mi++] = y+1; }
            if( src->get(x-1,y) == 0xFFFFFF ) { M[Mi++] = x-1; M[Mi++] = y; }
            if( src->get(x,y-1) == 0xFFFFFF ) { M[Mi++] = x; M[Mi++] = y-1; }
         }
         w = x2-x1+1; h = y2-y1+1;
      };
      block( bmp *pbmp ) : src(pbmp) {};
      block( bmp *pbmp, DWORD x, DWORD y ) : src(pbmp) { suck( x, y ); };
      block( char *fname ){ // TODO: get the biggest not the first
         src = new bmp( fname );
         for( DWORD y=src->h-1; y>0; y-- )
            for( DWORD x=0; x<src->w; x++ )
               if( src->get(x,y)==0xFFFFFF )
                  { suck(x,y); x=src->w; y=src->h; }
      }
      bool get( int x, int y ){
         if( x<0 || y<0 || x>w-1 || y>h-1 ) return false;
         return src->get( x1+x, y1+y ) == id;
      }
      void draw(){
         for( DWORD y=0; y<h; y++ ){
            for( DWORD x=0; x<w; x++ )
               printf( "%c", (get(x,y)?'b':' ') );
            printf( "\n" );
         }
      }
   };
      
   double cmp( block *a, block *b ){
      DWORD eq=0;
      for( DWORD y=0; y<=a->y2-a->y1; y++ )
         for( DWORD x=0; x<=a->x2-a->x1; x++ )
            if( a->get(x,y) && b->get(x,y) ) eq+=2;
      return ((double)eq)/((double)(a->m+b->m-eq));
   }

   bool alike( double mould, double sample, double toleranceQ ){
      return (sample<mould+mould*toleranceQ)&&(sample>mould-mould*toleranceQ);
   }

   bool rgz( block *x=0, int fonts=0xF ){
      static block* G [1000]; DWORD Gi=0;
      static bool on = false;
      if( !on ){
         for( ; Gi<1000; Gi++ ) G[Gi]=0; Gi=0;
         char s [200];
         for( int fn=1; fn<5; fn++ ){
            printf( " Font%i ", fn ); sprintf( s, "internals\\glyphs\\%d*", fn );
            WIN32_FIND_DATA r; HANDLE h = FindFirstFile( s, &r ); if( h == INVALID_HANDLE_VALUE ) die(1);
            do{
               if( !(r.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && strlen(r.cFileName)==11 ){
                  sprintf( s, "internals\\glyphs\\%s", r.cFileName );
                  G[Gi] = new block( s );
                  G[Gi]->fn = r.cFileName[0]-48;
                  G[Gi]->v[0] = (r.cFileName[1]-48)*100+(r.cFileName[2]-48)*10+(r.cFileName[3]-48);
                  G[Gi]->v[1] = (r.cFileName[4]-48)*100+(r.cFileName[5]-48)*10+(r.cFileName[6]-48);
                  printf( "%c", G[Gi]->v[0] );
                  Gi++;
               }
            } while( FindNextFile( h, &r) );
            printf( "\n" );
         }
         printf( "\n" );
         if( Gi==0 ) die(1);
         on = true;
         Gi=0;
      };
      if(x) {  
         double q, maxq=-1; int maxqi=-1;
         for( ; G[Gi]; Gi++ )
            if( (fonts&(1<<(G[Gi]->fn-1))) && alike( G[Gi]->m, x->m, 0.5 ) && alike( G[Gi]->w, x->w, 0.35 ) && alike( G[Gi]->h, x->h, 0.35 ) ) {
               q = cmp( G[Gi], x );
               if( q > maxq ){ maxq = q; maxqi=Gi; }
            }
         if( maxqi == -1 || (fonts!=8&&maxq<0.5) ) return false;
         x->fn = G[maxqi]->fn;
         x->v[0] = G[maxqi]->v[0];
         x->v[1] = G[maxqi]->v[1];
         return true;
      }else return false;
   }

   // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- - -- -

   bmp page;
   block vl( &page ), hl( &page );
   double rot, vll;

   bool getvline(){
      DWORD x,y;
      for( y=0; y<page.h; y++ ) for( x=0; x<page.w; x++ )
         if( page.get(x,y)==0xFFFFFF ){
            vl.suck(x,y);
            if( vl.m>20 && vl.w!=0 && vl.h/vl.w > 40 ) return true;
         }
      return false;
   }

   bool gethline( int right=0 ){
      DWORD y;
      for( y=0; y<page.h; y++ )
         if( page.get( 1000+right*2000, y ) == 0xFFFFFF ){
            hl.suck( 1000+right*2000, y );
            if( hl.m>20 && hl.w!=0 && hl.w/hl.h > 40 ) return true;
         }
      return false;
   }

   void getrotnlen(){
      double ax=0,ay=0,bx=0,by=0; DWORD n=0,x,y;
      for( y=0; y<5; y++ ) for( x=0; x<vl.w; x++ ) if( vl.get(x,y) )
         { ax += x; ay += y; n++; }
      ax /= n; ay /= n; n = 0;
      for( y=vl.h-5; y<vl.h; y++ ) for( x=0; x<vl.w; x++ ) if( vl.get(x,y) )
         { bx += x; by += y; n++; }
      bx /= n; by /= n;
      rot = atan((bx-ax)/(by-ay))*180/3.14159265;
      vll = sqrt( (bx-ax)*(bx-ax) + (by-ay)*(by-ay) );
   }

   // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- - -- -

   char s[200]="";
   char A[200]="", T[200]="", S[200]="";
   char P1[10]="", P2[10]="", P3[10]="";
   double p1, p2, p3;

   void escape( char* str ){
         char tmp [200]; int tmpi=0;
         for( int i=0; i<strlen(str); i++ ){
            tmp[tmpi++]=str[i];
            if( str[i]=='"' ) tmp[tmpi++]='"';
         }
         tmp[tmpi]=0;
         strcpy( str, tmp );
   }

   void getstudio( int x, int y ){
      S[0]=0;
      block c1( &page ), c2( &page );
      bool found = false;
      for( ; !found && x<hl.x2-500; x++ )
         if( page.get( x,y ) == 0xFFFFFF ){
            c1.suck( x,y );
            if( rgz( &c1, 4 ) ) found=true;
         }
      strcpy( S, c1.v );
      y = c1.y2;
      int ymin=c1.y1;
      x=c1.x2;
      for( ; x<hl.x2-500; x++ )
         for( int yy=y; yy>ymin; yy-- )
            if( page.get( x,yy ) == 0xFFFFFF ){
               c2.suck( x,yy );
               if( rgz( &c2 , 4 ) ){
                  if( c2.y2>y ) y=c2.y2;
                  if( c2.y1<ymin ) ymin=c2.y1;
                  if( c2.x1-c1.x2 > 15 ) strcat( S, " " );
                  strcat( S, c2.v );
                  c1=c2;
               }
            }
   }

   void getprices( int x, int y ){
      P1[0]=P2[0]=P3[0]=0;
      block c( &page );
      bool found=false;
      for( ; !found && x<hl.x2; x++ )
         if( page.get( x,y ) == 0xFFFFFF ){
            c.suck( x,y );
            if( rgz( &c, 8 ) ) found=true;
         }
      strcpy( P1, c.v );
      y=c.y2;
      int ymin=c.y1;
      x = c.x2;
      for( ; x<hl.x2; x++ )
         for( int yy=y; yy>ymin; yy-- )
            if( page.get( x,yy ) == 0xFFFFFF ){
               c.suck( x,yy );
               if( rgz( &c, 8 ) ){
                  if( x<hl.x2-320 ) strcat( P1, c.v );
                  else if( x<hl.x2-150 ) strcat( P2, c.v );
                  else strcat( P3, c.v );
                  x = c.x2;
               }
            }
      sprintf( s, "%s %s %s", P1, P2, P3 );
      if( sscanf( s, "%f %f %f", &p1, &p2, &p3 )==3 ){
         // TODO: fix
      }
   }

   void save(){
      FILE*f;
      if( *A==0 ){
         f = fopen( "internals\\artist.txt", "r" );
         if( f ){
            fseek( f, 0, SEEK_END );
            int i=ftell(f);
            fseek( f, 0, SEEK_SET );
            fread( A, 1, i, f );
            A[i]=0;
            printf( " (%s) mem\n\n", A );
            fclose( f );
         }
      }
      f = fopen( "OUTPUT.csv", "a" ); if( !f ) die(5);
      escape( A ); escape( T ); escape( S );
      sprintf( s, "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n", A, T, S, P1, P2, P3 );
      fwrite( s, 1, strlen(s), f );
      fclose( f );
      f = fopen( "internals\\artist.txt", "w" ); fwrite( A, 1, strlen(A), f ); fclose( f );
      printf( " %s\n%*s %*s %*s %*s\n", T, 56, S, 7, P1, 7, P2, 7, P3 );
      *T=*S=*P1=*P2=*P3=0;
   }

   void getpart(){
      int y = hl.y2; 
      while( y < vl.y2+100 ){
         int x = hl.x1 -20;
         y += 20;
         block c1( &page );
         bool found = false;
         for( ; !found && x<hl.x1+100; x++ ) if( page.get(x,y) == 0xFFFFFF ){
            c1.suck(x,y);
            if( rgz( &c1, 3 ) && ((c1.v[0]>47&&c1.v[0]<58)||(c1.v[0]>64&&c1.v[0]<91)) ) found=true;
         }
         if( !found ) continue;
         if( y < c1.y2 ) y = c1.y2;
         int ymin = c1.y1;
         strcpy( s, c1.v );
         block c2=c1, c3( &page );
         bool stop1=false, stop2=false;
         for( ; !stop1 && !stop2; x++ ){ 
            for( int yy=y; yy>ymin; yy-- )
               if( page.get( x, yy ) == 0xFFFFFF ){
                  c3.suck( x, yy );
                  if( rgz( &c3, c1.fn ) ){
                     if( c3.y2>y ) y=c3.y2;
                     if( c3.y1<ymin ) ymin=c3.y1;
                     if( ((int)(c3.x1))-((int)(c2.x2)) > 15 ) { strcat( s, " " ); }
                     if( c3.v[0]==44 && ((c3.y1+c3.y2)/2)<((y+ymin)/2) ) strcat( s, "'" );
                     else if( c3.v[0]==39 && ((c3.y1+c3.y2)/2)>((y+ymin)/2) ) strcat( s, "," );
                     else if( c3.v[0]=='"' && c2.v[0]=='"' );
                     else strcat( s, c3.v );
                     c2=c3;
                  }
               }
            if( x>c3.x2+150 ) { stop1=true; }
            if( c3.v[0] == '.' && c2.v[0] == '.' ) { stop2=true; s[strlen(s)-2]=0; }
         }
         switch( c1.fn ){
            case 1:
               strcpy( A, s );
               printf( " (%s)\n\n", A );
               break;
            case 2:
               if( stop1 ) { strcpy( T, s ); strcat( T, " " ); }
               else{
                  strcat( T, s );
                  getstudio( x, (y+ymin)/2 );
                  getprices( x, (y+ymin)/2 );
                  save();
               }
               break;
         }
      }
   }

   int main(){ printf( "\n %s", Title );

      { FILE *f = fopen( "internals\\convert.exe", "rb" ); if( !f ) die(1); fclose( f ); }

      rgz();

      WIN32_FIND_DATA r; HANDLE h = FindFirstFile( "INPUT\\*", &r ); if( h == INVALID_HANDLE_VALUE ) die(1); int nf=0;
      do if( !(r.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && r.cFileName[0] != '.' ){ nf++;
            
            printf( " File: %s\n", r.cFileName ); sprintf( s, "internals\\convert.exe INPUT\\%s -type truecolor -depth 8 internals\\page.bmp", r.cFileName ); system( s );
            printf( " L" ); page.load( "internals\\page.bmp" );

            printf( "V" ); getvline();
            printf( "D" ); getrotnlen();
            
            printf( "R" ); sprintf( s, "internals\\convert.exe INPUT\\%s -type truecolor -depth 8 -rotate %5.4f -resize %5.4f%% internals\\page.bmp", r.cFileName, rot, 550000/vll ); system( s );
            printf( "L" ); page.load( "internals\\page.bmp" );

            printf( "V" ); getvline();
            printf( "H\n" ); gethline();

            getpart();
            
            page.load( "internals\\page.bmp" );
            gethline(1);

            getpart();

            sprintf( s, "INPUT\\%s", r.cFileName );
            sprintf( s+250, "INPUT\\processed\\%s", r.cFileName );
            rename( s, s+250 );
            
            printf( "\n" );

      } while( FindNextFile( h, &r ) );
      FindClose(h);

      printf( " %d Files processed\n\n", nf );

   die(0); }
