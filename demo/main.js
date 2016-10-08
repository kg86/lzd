// ===========================================
var CHAR_SIZE = 256;
var CHAR_PX = 10;

var CHAR_WIDTH = 2*CHAR_PX; // default:2
var CHAR_HEIGHT = CHAR_WIDTH;
var CharFont= CHAR_PX + "px 'Lucida Console'";
var CharFont= CHAR_PX + "px 'Arial'";
var MarginChar = CHAR_WIDTH + CHAR_PX / 2;
var lzd = function(text){
    var dic = new Array();
    for (var i = 0; i < CHAR_SIZE; i++){
        dic[String.fromCharCode(i)] = i; // text.charCodeAt
    }
    var prev = -1;
    var previ = 0;
    var seq = new Array();
    var i = 0;
    var vIdx = CHAR_SIZE;
    while (i < text.length){
        var len = text.length - i;
        while (len > 1 && !(text.substr(i, len) in dic)) len--;
        // console.log('i=' + i + ' len=' + len + ' str=' + text.substr(i, len));
        if (prev == -1){
            prev = dic[text.substr(i, len)];
            // console.log('i=' + i + ' prev=' + prev);
            previ = i;
            i = i + len;
        }else{
            var cur = dic[text.substr(i, len)];
            dic[text.substring(previ, i+len)] = vIdx;
            vIdx++;
            // console.log('prev=' + prev + ' cur='+ cur);
            seq[seq.length] = new Array(prev, cur);
            prev = -1;
            previ = -1;
            i = i + len;
        }
    }
    if (prev != -1){
        seq[seq.length] = new Array(prev, -1);
    }
    return seq;
};
var lz78 = function(text){
    console.log('LZ78');
    var dic = new Array();
    // for (var i = 0; i < CHAR_SIZE; i++){
    //     dic[String.fromCharCode(i)] = i; // text.charCodeAt
    // }
    var seq = new Array();
    var i = 0;
    var vIdx = CHAR_SIZE;
    while (i < text.length){
        var len = text.length - i;
        while (len > 0 && !(text.substr(i, len) in dic)) len--;
        // console.log('i=' + i + ' len=' + len + ' str=' + text.substr(i, len));
        var prev = dic[text.substr(i, len)];
        if (len == 0){
            // first occurrence
            dic[text.substr(i, 1)] = text.charCodeAt(i);
            seq[seq.length] = new Array(-1, text.charCodeAt(i));
        }else if (i + len >= text.length) {
            seq[seq.length] = new Array(prev, -1);
        } else {
            var next = text.substr(i+len, 1).charCodeAt(0);
            seq[seq.length] = new Array(prev, next);
            dic[text.substr(i, len+1)] = vIdx;
        }
        vIdx++;
        i += len + 1;
    }
    return seq;
};


var FTree = function(text, seq){
    this.text = text;
    this.seq = seq;
};

FTree.drawEdge = function(ctx, par, child){
    ctx.beginPath();
    p_bottom = par.bottom();
    c_top = child.top();
    ctx.moveTo(p_bottom.x, p_bottom.y);
    ctx.lineTo(c_top.x, c_top.y);
    ctx.stroke();
};

FTree.prototype.drawTree = function(context, vIdx, offX, offY){
    if (vIdx < CHAR_SIZE){
        var cur = new charNode(String.fromCharCode(vIdx), offX, offY);
        cur.draw(context);
        var p = cur.top();
        return new TreeSize(cur, offX, offY, 0, 0);
    }else if (this.seq[vIdx-CHAR_SIZE][0] == -1){
        // charnode
        return this.drawTree(context, this.seq[vIdx-CHAR_SIZE][1], offX, offY);
    }
    console.log('vIdx(' + vIdx + ')=[' + this.seq[vIdx-CHAR_SIZE][0]
                + ']['+ this.seq[vIdx-CHAR_SIZE][1] + ']');
    var leftt = this.drawTree(context, this.seq[vIdx-CHAR_SIZE][0], offX, offY);
    var rightt = this.drawTree(context, this.seq[vIdx-CHAR_SIZE][1], offX + leftt.w + MarginChar, offY);
    var cur = new charNode(vIdx, (leftt.root.x + rightt.root.x) / 2,
                           Math.min(leftt.root.y, rightt.root.y) - MarginChar);
    cur.draw(context);
    FTree.drawEdge(context, cur, leftt.root);
    FTree.drawEdge(context, cur, rightt.root);
    return new TreeSize(cur, offX, offY, leftt.w + MarginChar + rightt.w,
                        Math.max(leftt.h, rightt.h) + MarginChar);
};

var TreeSize = function(root, offX, offY, w, h){
    this.root = root;
    this.offX = offX;
    this.offY = offY;
    this.w = w;
    this.h = h;
};


var Point = function(x, y){
    this.x = x;
    this.y = y;
};

var charNode = function(c, x, y, w, h){
    this.c = c;
    this.x = x;
    this.y = y;
    if (w == undefined) this.w = CHAR_WIDTH;
    else this.w = w;
    if (h == undefined) this.h = CHAR_HEIGHT;
    else this.h = h;
};

charNode.prototype.top = function(){
    return new Point(this.x, this.y - this.h*2/3);
};
charNode.prototype.bottom = function(){
    return new Point(this.x, this.y + this.h/3);
};

charNode.prototype.draw = function(context){
    context.textAlign = "center";
    context.fillStyle = 'black';
    context.fillRect(this.x - this.w/2, this.y - this.h * 2/3, this.w, this.h);
    context.fillStyle = 'white';
    context.fillRect(this.x - this.w/2+1, 1+this.y - this.h * 2/3, this.w-2, this.h-2);
    context.fillStyle = 'black';
    context.fillText(this.c, this.x, this.y);
};

function drawText()
{
    var text = document.getElementById('CW').value;
    console.log("*************** START ******************");
    console.log(text);

    Radius = 3; // 内部ノードの半径
    UnitW   = 15;
    UnitH   = 15;
    var OffX    = 30;
    var OffY = 100;
    OffH    = 50;
    BoxW    = 6;
    BoxH    = 6;
    CharMarginX = 1; // margin to locate char in box properly
    CharMarginY = 10;// margin to locate char in box properly
    LineWidthHalf = 0.5;


    // var MarginChar = 40;

    var cv = document.getElementById("TREE"); 
    if (! cv || ! cv.getContext ) {  return false; }

    var context = cv.getContext("2d");
    context.clearRect(0, 0, cv.width, cv.height);
    context.font = CharFont;


    // console.log(text.length);
    var w = 30;
    var h = w;

    var ffseq = lzd(text);
    var lz78seq = lz78(text);
    var fftree = new FTree(text, ffseq);
    var lz78tree = new FTree(text, lz78seq);
    var x = OffX;
    var y = OffY;
    for (var i = 0; i < ffseq.length; i++){
        var cur_tree = fftree.drawTree(context, i + CHAR_SIZE, x, y+100);
        x += cur_tree.w + MarginChar;
    }
    x = OffX;
    y = OffY;
    for (var i = 0; i < lz78seq.length; i++){
        var cur_tree = lz78tree.drawTree(context, i + CHAR_SIZE, x, y+300);
        x += cur_tree.w + MarginChar;
    }
};
// ===========================================

function clearTree(cv)
{
}
// ===========================================







