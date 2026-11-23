// Swiss tournaments on the M9 screen (ChessTour.h). Included by Ui.cpp after UiChess.inc, inside its
// anonymous namespace. The "Tournaments" tab lists them; a tournament page shows my game of the round,
// the standings and the games; the organiser creates one in four steps (name, players, rules, check).
tour::Tour* tourOpen=nullptr;
int tourSel=0,tourView=0,tourRow=0,tourStep=0,tourRounds=0,tourHoursAt=2;
bool tourPicked[24]={};String tourName;uint32_t tourEventsSeen=0;
const uint8_t tourHours[]={6,12,24,48,72};
const uint16_t tourViolet=rgb(0x2e2450),tourOkBg=rgb(0x1d3b26);

int tourPill(int x,int y,const String& s,uint16_t fg,uint16_t bgc){int w=measure(s,small)+8;g().fillRoundRect(x,y,w,11,3,bgc);text(x+4,y+8,s,fg,small);return w;}
unsigned tourRows(tour::Tour** out){
 unsigned n=0;for(auto& T:tour::net.tours)if(T.state!=tour::Free)out[n++]=&T;
 auto rank=[](const tour::Tour* T){return T->state==tour::Invited||T->unseen?0:T->active()?1:2;};
 for(unsigned i=1;i<n;i++)for(unsigned j=i;j>0&&(rank(out[j])<rank(out[j-1])||(rank(out[j])==rank(out[j-1])&&int32_t(out[j]->changedAt-out[j-1]->changedAt)>0));j--)std::swap(out[j],out[j-1]);
 return n;
}
// My place and points, from the standings this device has.
bool tourPlace(const tour::Tour& T,unsigned& place,unsigned& of,uint8_t& half){
 tour::Row rows[tour::MaxPlayers];of=tour::net.standings(T,rows);for(unsigned i=0;i<of;i++)if(rows[i].me){place=i+1;half=rows[i].half;return true;}return false;
}
String tourPoints(uint8_t half){return String(half/2)+(half&1?".5":"");}
ChessMatch* tourGameOf(const tour::Tour& T,const tour::Pair& p){uint64_t op=tour::net.opponent(T,p);if(!op)return nullptr;for(auto& m:chessNet.matches)if(m.state!=ChessMatch::Free&&m.id==p.game&&m.tour==T.id)return &m;return nullptr;}
String tourStateLine(const tour::Tour& T){
 switch(T.state){
 case tour::Invited:return tr("From ")+String(T.nameOf(T.organizer))+" · "+count(T.rounds,"round","rounds")+" · "+String(T.hours)+tr(" h a move");
 case tour::Inviting:{unsigned joined=0;for(unsigned i=0;i<T.playerCount;i++)joined+=T.players[i].state==tour::Joined;return t("Agreed: ")+String(joined)+tr(" of ")+String(T.playerCount);}
 case tour::Declined:return t("You declined");
 case tour::Cancelled:return t("Cancelled");
 default:break;
 }
 unsigned place=0,of=0;uint8_t half=0;String s;
 if(T.state==tour::Over){if(tourPlace(T,place,of,half))return String(place)+t(" place of ")+String(of)+" · "+tourPoints(half)+t(" pts");return t("Finished");}
 if(!T.round)return t("Waiting for the start");
 s=t("Round ")+String(T.round)+tr(" of ")+String(T.rounds);
 if(tourPlace(T,place,of,half)&&T.standCount+T.organising())s+=" · "+String(place)+t(" place, ")+tourPoints(half)+t(" pts");
 if(const tour::Pair* p=tour::net.myPair(T,T.round)){ChessMatch* m=tourGameOf(T,*p);if(tour::net.myColor(T,*p)==2)s+=t(" · a bye");else if(m)s+=" · "+String(m->myTurn()?t("your move"):m->state==ChessMatch::Over?t("game over"):t("their move"));}
 return s;
}
void tourStatePill(int x,int y,const tour::Tour& T){
 switch(T.state){case tour::Running:tourPill(x,y,T.round?t("on"):t("starting"),ok,tourOkBg);break;case tour::Invited:tourPill(x,y,t("invitation"),accent,rgb(0x14524b));break;
 case tour::Inviting:tourPill(x,y,t("gathering"),warn,rgb(0x3b3016));break;case tour::Over:tourPill(x,y,t("finished"),dim,line);break;default:tourPill(x,y,t("off"),faint,line);}
}
void drawTourList(){
 tour::Tour* rows[tour::MaxTours];unsigned n=tourRows(rows),total=1+n;if(tourSel>=int(total))tourSel=total-1;
 for(unsigned i=0;i<total;i++){int y=46+i*44;bool focus=tourSel==int(i);listRow(y,42,focus);target(8,y,304,42,i);
  if(!i){g().fillCircle(32,y+21,14,rgb(0x14524b));g().fillRect(31,y+13,3,17,accent);g().fillRect(24,y+20,17,3,accent);text(56,y+17,t("Create a tournament"),ink,bold);text(56,y+32,t("Swiss system, you organise and play"),dim,small);continue;}
  const tour::Tour& T=*rows[i-1];g().fillCircle(32,y+21,14,T.active()?tourViolet:card);icon(IcChess,32,y+21,8,T.active()?violet:faint,T.active()?tourViolet:card);
  String name=fit(T.name,150,bold);text(56,y+17,name,ink,bold);tourStatePill(60+measure(name,bold),y+7,T);
  if(T.playerCount)textRight(304,y+16,count(T.playerCount,"player","players"),faint,small);
  text(56,y+32,fit(tourStateLine(T),236,small),T.state==tour::Invited?accent:dim,small);if(T.unseen||T.state==tour::Invited)g().fillCircle(300,y+30,4,accent);
 }
 if(!n){String r[3];unsigned k=wrap(t("Players get their pairings, games and standings as MeshCore messages. Pairings are automatic: round 1 by lot, then by score."),r,3,296);for(unsigned i=0;i<k;i++)textCenter(160,120+i*16,r[i],faint);}
 footer({{"OK",tourSel?t("Open"):t("Create")},{"<>",t("Tabs")},{"BACK",t("Menu")}});
}
void tourOpenCard(tour::Tour* T){tourOpen=T;tourView=0;tourRow=0;tour::net.viewed(*T);change(ChessTour);}
void tourStartWizard(){tourStep=0;tourRounds=0;tourHoursAt=2;memset(tourPicked,0,sizeof tourPicked);tm v;tourName=t("Tournament");if(localTime(time(nullptr),v)){char b[24];snprintf(b,sizeof b," %02d.%02d",v.tm_mday,v.tm_mon+1);tourName+=b;}change(ChessTourNew);}
bool tourListKey(int key){
 tour::Tour* rows[tour::MaxTours];unsigned n=tourRows(rows),total=1+n;
 if(key==KeyUp||key==KeyDown){tourSel=(tourSel+(key==KeyUp?-1:1)+total)%total;return true;}
 if(key==Enter||key==KeyHold){if(!tourSel)tourStartWizard();else tourOpenCard(rows[tourSel-1]);return true;}
 if(key==KeyBack){change(Home,true);return true;}
 return false;
}

// The tournament page.
void tourDots(int x,int y,const tour::Tour& T){for(unsigned i=0;i<T.rounds;i++){int cx=x+i*12;if(i+1<T.round||T.state==tour::Over)g().fillCircle(cx,y,4,accent);else if(i+1==T.round){g().drawCircle(cx,y,4,accent);g().fillCircle(cx,y,2,accent);}else g().drawCircle(cx,y,4,faint);}}
void drawTourInvite(const tour::Tour& T){
 panel(8,24,304,190,cardHi,10);g().fillCircle(32,48,13,tourViolet);icon(IcChess,32,48,7,violet,tourViolet);
 text(54,44,T.state==tour::Invited?t("Tournament invitation"):t("You declined"),dim,small);text(54,58,fit(T.name,250,bold),ink,bold);
 String k[]={t("Organiser"),t("Format"),t("Time a move"),t("Rating")};
 String v[]={T.nameOf(T.organizer),t("Swiss, ")+count(T.rounds,"round","rounds"),String(T.hours)+t(" h"),t("ELO counts")};
 for(int i=0;i<4;i++){text(24,92+i*20,k[i],dim,small);text(120,92+i*20,fit(v[i],180),ink);}
 String r[2];unsigned n=wrap(t("Pairings come automatically; each game is an ordinary rated game."),r,2,280,small);for(unsigned i=0;i<n;i++)text(24,182+i*11,r[i],faint,small);
 if(T.state==tour::Invited)footer({{"OK",t("Join")},{"DEL",t("Decline")},{"BACK",t("Later")}});else footer({{"DEL",t("Remove")},{"BACK",t("Back")}});
}
void drawTourGathering(const tour::Tour& T){
 panel(8,24,304,22,card,6);text(16,38,t("Invitations sent"),ink,bold);textRight(304,38,count(T.rounds,"round","rounds")+" · "+String(T.hours)+t(" h a move"),dim,small);
 for(unsigned i=0;i<T.playerCount&&i<8;i++){const tour::Player& p=T.players[i];int y=50+i*20;avatar(20,y+9,8,p.id,p.name,1);text(34,y+13,fit(p.name,180),ink);
  textRight(304,y+13,i==0?t("organiser"):p.state==tour::Joined?t("joins"):p.state==tour::Refused?t("declined"):t("no answer yet"),p.state==tour::Joined?ok:p.state==tour::Refused?bad:faint,small);}
 text(16,212,fit(t("Starts by itself when everyone answers; S starts with those who agreed."),296,small),faint,small);
 footer({{"S",t("Start")},{"DEL",t("Cancel")},{"BACK",t("Back")}});
}
void drawTourCard(){
 if(!tourOpen||tourOpen->state==tour::Free){textCenter(160,110,t("The tournament was removed"),dim);footer({{"BACK",t("Tournaments")}});return;}
 const tour::Tour& T=*tourOpen;
 if(T.state==tour::Invited||T.state==tour::Declined){drawTourInvite(T);return;}
 if(T.state==tour::Inviting){drawTourGathering(T);return;}
 panel(8,24,304,22,card,6);text(16,38,T.round?t("Round ")+String(T.round)+tr(" of ")+String(T.rounds):t("Waiting for the start"),ink,bold);tourDots(124,35,T);
 textRight(304,38,T.state==tour::Over?t("finished"):T.state==tour::Cancelled?t("cancelled"):String(T.hours)+t(" h a move"),dim,small);
 // My game of the round.
 const tour::Pair* p=tour::net.myPair(T,T.round);int y=50;
 if(p&&T.state==tour::Running){panel(8,y,304,40,cardHi,7);ring(8,y,304,40,7,accent);uint8_t c=tour::net.myColor(T,*p);
  if(c==2){text(18,y+17,t("A bye this round"),ink,bold);text(18,y+32,t("One point without a game: odd number of players"),dim,small);}
  else{uint64_t op=tour::net.opponent(T,*p);ChessMatch* m=tourGameOf(T,*p);avatar(30,y+20,13,op,T.nameOf(op),1);colorDisc(41,y+31,c);
   text(52,y+16,fit(t("Round ")+String(T.round)+": "+T.nameOf(op),210,bold),ink,bold);
   text(52,y+31,fit(m?chessState(*m)+(m->game.plies&&m->state!=ChessMatch::Invited?" · "+lastSan(*m):String()):String(t("no board free")),210,small),m?chessStateColor(*m):bad,small);
   if(m){g().fillRoundRect(276,y+11,28,17,4,accent);textCenter(290,y+23,"OK",bg,small);}}
  y+=44;}
 else if(T.state==tour::Running){text(16,y+14,t("Waiting for the next round's pairing"),faint,small);y+=20;}
 if(tourView==0){ // standings
  tour::Row rows[tour::MaxPlayers];unsigned n=tour::net.standings(T,rows);
  text(16,y+10,"#",faint,small);text(32,y+10,t("Player"),faint,small);textRight(212,y+10,t("Pts"),faint,small);textRight(258,y+10,t("Buch."),faint,small);textRight(304,y+10,"ELO",faint,small);
  int visible=(216-(y+14))/14;if(tourRow>int(n)-visible)tourRow=max(0,int(n)-visible);
  for(int i=tourRow;i<int(n)&&i<tourRow+visible;i++){const tour::Row& r=rows[i];int ry=y+14+(i-tourRow)*14;if(r.me){panel(8,ry,304,14,cardHi,4);g().fillRoundRect(8,ry+3,3,8,1,accent);}
   text(16,ry+11,String(i+1),r.me?ink:dim,small);text(32,ry+11,fit(r.me?String(t("You")):String(r.name),160,r.me?bold:body),ink,r.me?bold:body);
   textRight(212,ry+11,tourPoints(r.half),ink,bold);textRight(258,ry+11,tourPoints(r.buch),dim);textRight(304,ry+11,String(rating::book.elo(r.id)),dim);}
  scrollbar(tourRow,visible,n,y+14,visible*14);
 }else{ // games: all of the round for the organiser, my own for a player
  int row=0;
  if(T.organising()&&T.round){unsigned r=T.round-1;text(16,y+10,t("Round ")+String(T.round)+t(" games"),faint,small);
   for(unsigned k=0;k<T.pairCount[r]&&row<7;k++,row++){const tour::Pair& q=T.pairs[r][k];int ry=y+14+row*20;bool focus=tourRow==int(k);listRow(ry,18,focus);
    colorDisc(20,ry+9,0);text(30,ry+13,fit(T.players[q.white].name,100),ink);
    if(q.black<tour::MaxPlayers){colorDisc(150,ry+9,1);text(160,ry+13,fit(T.players[q.black].name,90),ink);}
    String res=q.result==tour::WhiteWon?"1-0":q.result==tour::BlackWon?"0-1":q.result==tour::Drawn?"1/2":q.result==tour::Bye?"+1":q.result==tour::Disputed?t("dispute"):"-";
    textRight(304,ry+13,res,q.result==tour::Disputed?bad:q.result==tour::Pending?faint:ink,bold);}}
  else{text(16,y+10,t("Your games"),faint,small);
   for(unsigned r=1;r<=T.round&&row<7;r++){const tour::Pair* q=tour::net.myPair(T,r);if(!q)continue;int ry=y+14+row*20;row++;uint8_t c=tour::net.myColor(T,*q);
    text(16,ry+13,t("R")+String(r),faint,small);
    if(c==2){text(44,ry+13,t("bye (+1)"),dim);continue;}
    colorDisc(50,ry+9,c);text(60,ry+13,fit(T.nameOf(tour::net.opponent(T,*q)),170),ink);
    bool won=(q->result==tour::WhiteWon&&c==0)||(q->result==tour::BlackWon&&c==1),lost=(q->result==tour::WhiteWon&&c==1)||(q->result==tour::BlackWon&&c==0);
    textRight(304,ry+13,won?t("win"):lost?t("loss"):q->result==tour::Drawn?t("draw"):t("playing"),won?ok:lost?bad:dim,small);}}
 }
 bool dispute=tourView==1&&T.organising()&&T.round&&tourRow<int(T.pairCount[T.round-1])&&T.pairs[T.round-1][tourRow].result==tour::Disputed;
 if(dispute)footer({{"W",t("1-0")},{"B","0-1"},{"D","1/2"},{"BACK",t("Back")}});
 else if(T.active())footer({{"OK",t("To the game")},{"<>",tourView?t("Standings"):t("Games")},{"BACK",t("Back")}});
 else footer({{"<>",tourView?t("Standings"):t("Games")},{"DEL",t("Remove")},{"BACK",t("Back")}});
}
bool tourCardKey(int key){
 if(key>='A'&&key<='Z')key+=32;
 if(key==KeyBack){change(ChessList);chessTab=1;return true;}
 if(!tourOpen||tourOpen->state==tour::Free)return false;
 tour::Tour& T=*tourOpen;
 if(T.state==tour::Invited){
  if(key==Enter||key==KeyHold){if(tour::net.accept(T))notice(t("You joined: wait for the pairing"),ok);return true;}
  if(key==Erase){if(tour::net.decline(T))notice(t("Invitation declined"),dim);return true;}
  return false;
 }
 if(T.state==tour::Inviting){
  if(key=='s'){if(tour::net.start(T))notice(t("Tournament started: round 1 sent"),ok);else notice(tour::net.event,bad);return true;}
  if(key==Erase){if(tour::net.cancel(T))notice(t("Tournament cancelled"),dim);return true;}
  return false;
 }
 if(key==KeyLeft||key==KeyRight){tourView^=1;tourRow=0;return true;}
 if(key==KeyUp||key==KeyDown){tourRow=max(0,tourRow+(key==KeyUp?-1:1));return true;}
 if(tourView==1&&T.organising()&&(key=='w'||key=='b'||key=='d')&&T.round&&tourRow<int(T.pairCount[T.round-1])){
  const tour::Pair& q=T.pairs[T.round-1][tourRow];if(q.result==tour::Disputed&&tour::net.setResult(T,q.game,key=='w'?tour::WhiteWon:key=='b'?tour::BlackWon:tour::Drawn))notice(t("Result set"),ok);return true;}
 if(key==Enter||key==KeyHold){const tour::Pair* p=tour::net.myPair(T,T.round);ChessMatch* m=p?tourGameOf(T,*p):nullptr;if(m){chessEnter(m);return true;}notice(t("No game for you now"),dim);return true;}
 if(key==Erase&&!T.active()){tour::net.remove(T);tourOpen=nullptr;change(ChessList);chessTab=1;notice(t("Tournament removed"),ok);return true;}
 return false;
}

// Creating a tournament: name, players, rules, check.
unsigned tourContacts(unsigned* out){unsigned n=0;for(unsigned i=0;i<meshRadio.peerCount&&n<24;i++)if(meshRadio.peers[i].type==1)out[n++]=i;return n;}
unsigned tourPickedCount(){unsigned c[24],n=tourContacts(c),k=0;for(unsigned i=0;i<n;i++)k+=tourPicked[i];return k;}
int tourSuggested(unsigned players){int r=1;while((1<<r)<int(players))r++;return min(int(tour::MaxRounds),min(int(players)-1+ (players<3),r+1));}
void tourSteps(){const char* n[4]={tr("Name"),tr("Players"),tr("Rules"),tr("Check")};for(int i=0;i<4;i++){int x=8+i*76;bool on=i==tourStep,done=i<tourStep;
 g().fillRoundRect(x,24,72,4,2,done||on?accent:line);text(x,40,n[i],on?ink:done?dim:faint,on?bold:small);}}
void drawTourNew(){
 tourSteps();unsigned picked=tourPickedCount();
 if(tourStep==0){panel(8,50,304,44,cardHi,7);text(18,66,t("Tournament name"),dim,small);text(18,84,fit(tourName,280,bold),ink,bold);
  String r[3];unsigned n=wrap(t("Players see it in the invitation and the standings. OK changes it, > goes on."),r,3,290,small);for(unsigned i=0;i<n;i++)text(16,112+i*12,r[i],faint,small);
  footer({{"OK",t("Edit")},{"<>",t("Next")},{"BACK",t("Cancel")}});return;}
 if(tourStep==1){unsigned c[24],n=tourContacts(c);if(selected>=int(n))selected=max(0,int(n)-1);int first=max(0,selected-3);
  if(!n)textCenter(160,120,t("No chat contacts yet"),dim);
  for(int i=first;i<int(n)&&i<first+4;i++){const Peer& p=meshRadio.peers[c[i]];int y=48+(i-first)*38;bool focus=selected==i;listRow(y,35,focus);target(8,y,304,35,i);
   bool on=tourPicked[i];if(on){g().fillRoundRect(18,y+10,15,15,3,accent);g().drawLine(21,y+17,24,y+21,bg);g().drawLine(24,y+21,30,y+13,bg);g().drawLine(21,y+18,24,y+22,bg);g().drawLine(24,y+22,30,y+14,bg);}else g().drawRoundRect(18,y+10,15,15,3,dim);
   avatar(52,y+17,12,p.id,p.name,1);text(72,y+15,fit(p.name,170,bold),ink,bold);bool fresh=p.heard&&millis()-p.seen<86400000;
   text(72,y+29,p.heard?ago(millis()-p.seen)+t(" ago"):String(t("not heard since the start")),fresh?dim:warn,small);textRight(304,y+17,"ELO "+String(rating::book.elo(p.id)),dim,small);}
  scrollbar(first,4,n,48,150);panel(8,202,304,16,card,4);
  text(16,213,fit(t("Chosen ")+String(picked+1)+t(" with you")+(picked>=tour::MaxPlayers-1?t(" (the most)"):String()),290,small),picked?dim:warn,small);
  footer({{"SPC",t("Choose")},{"OK",t("Next")},{"BACK",t("Back")}});return;}
 if(tourStep==2){if(!tourRounds)tourRounds=tourSuggested(picked+1);
  String keys[]={t("Rounds"),t("Time a move")},vals[]={String(tourRounds),String(tourHours[tourHoursAt])+t(" h")};
  String hints[]={t("suggested for ")+count(picked+1,"player","players")+": "+String(tourSuggested(picked+1)),t("a LoRa move takes up to a few minutes")};
  if(selected>1)selected=1;
  for(int i=0;i<2;i++){int y=50+i*36;bool focus=selected==i;listRow(y,33,focus);target(8,y,304,33,i);text(18,y+14,keys[i],ink,bold);text(18,y+27,hints[i],faint,small);
   int vw=measure(vals[i],bold);textRight(290,y+19,vals[i],focus?accent:ink,bold);if(focus){tri(290-vw-10,y+15,3,4,accent);tri(300,y+15,1,4,accent);}}
  String r[3];unsigned n=wrap(t("Every game is rated. Pairings are automatic: round 1 by lot, then by score without repeated games."),r,3,290,small);for(unsigned i=0;i<n;i++)text(16,136+i*12,r[i],faint,small);
  footer({{"<>",t("Value")},{"^v",t("Item")},{"OK",t("Next")},{"BACK",t("Back")}});return;}
 panel(8,48,304,62,cardHi,7);text(18,66,fit(tourName,280,bold),ink,bold);
 text(18,82,fit(t("Swiss · ")+count(tourRounds,"round","rounds")+" · "+String(tourHours[tourHoursAt])+t(" h a move")+" · ELO",290,small),dim,small);
 text(18,98,fit(count(picked+1,"player","players")+t(" with you"),290,small),dim,small);
 String r[4];unsigned n=wrap(t("Invitations go as direct messages. Round 1 starts when everyone answers, or when you press S on the tournament page; those who did not answer stay out."),r,4,290,small);
 for(unsigned i=0;i<n;i++)text(16,126+i*12,r[i],faint,small);
 footer({{"OK",t("Send invitations")},{"BACK",t("Back")}});
}
void tourNameSave(){
 if(!edit.length()||edit.length()>32||!meshmesh::validUtf8((const uint8_t*)edit.c_str(),edit.length())){notice(t("Name: 1-32 UTF-8 bytes"),bad);return;}
 tourName=edit;editing=false;tourStep=1;selected=0;
}
bool tourNewKey(int key){
 if(key>='A'&&key<='Z')key+=32;
 if(key==KeyBack){if(tourStep){tourStep--;selected=0;}else{change(ChessList);chessTab=1;}return true;}
 if(tourStep==0){if(key==Enter||key==KeyHold){editing=true;edit=tourName;return true;}if(key==KeyRight){tourStep=1;selected=0;return true;}return false;}
 if(tourStep==1){unsigned c[24],n=tourContacts(c);
  if(key==KeyUp||key==KeyDown){if(n)selected=(selected+(key==KeyUp?-1:1)+n)%n;return true;}
  if(key==' '||key==KeyRight){if(selected<int(n)){if(!tourPicked[selected]&&tourPickedCount()>=tour::MaxPlayers-1){notice(t("At most 10 players"),warn);return true;}tourPicked[selected]=!tourPicked[selected];}return true;}
  if(key==Enter||key==KeyHold){if(!tourPickedCount()){notice(t("Choose at least one contact"),warn);return true;}tourStep=2;tourRounds=0;selected=0;return true;}
  return false;}
 if(tourStep==2){
  if(key==KeyUp||key==KeyDown){selected^=1;return true;}
  if(key==KeyLeft||key==KeyRight){int d=key==KeyRight?1:-1;if(!selected)tourRounds=constrain(tourRounds+d,1,int(tour::MaxRounds));else tourHoursAt=constrain(tourHoursAt+d,0,4);return true;}
  if(key==Enter||key==KeyHold){tourStep=3;return true;}
  return false;}
 if(key==Enter||key==KeyHold){
  unsigned c[24],n=tourContacts(c);uint64_t ids[tour::MaxPlayers];unsigned k=0;for(unsigned i=0;i<n&&k<tour::MaxPlayers-1;i++)if(tourPicked[i])ids[k++]=meshRadio.peers[c[i]].id;
  tour::Tour* T=tour::net.create(tourName,tourRounds,tourHours[tourHoursAt],ids,k);
  if(!T){notice(tour::net.event,bad);return true;}tourOpenCard(T);notice(t("Invitations sent: ")+String(k),ok);return true;}
 return false;
}
// Tournament news: a toast, and a dark screen lights up.
void tourTick(){
 if(tour::net.events!=tourEventsSeen){tourEventsSeen=tour::net.events;
  if(tour::net.event.length()){lastInput=millis();if(wakeOnly){wakeOnly=false;hardware.brightness(config.brightness);}if(!locked)notice(tour::net.event,accent);}
  if(page==ChessTour&&tourOpen==tour::net.eventTour&&tourOpen)tour::net.viewed(*tourOpen);}
 if(tour::net.dirty){tour::net.dirty=false;dirty=true;}
}
void tourStatus(JsonDocument& d){
 if(page==ChessTour&&tourOpen){char id[5];snprintf(id,sizeof id,"%04X",tourOpen->id);d["tour"]=id;d["tour_view"]=tourView?"games":"standings";}
 if(page==ChessTourNew){d["tour_step"]=tourStep;d["tour_picked"]=tourPickedCount();}
}
