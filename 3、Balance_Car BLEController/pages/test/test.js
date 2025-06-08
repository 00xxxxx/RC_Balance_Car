// pages/test/test.js
const ecBLE = require('../../utils/ecBLE.js')

const RADIUS = 65
const MIN_DISTANCE = 5 //至少位移到5才触发摇杆

var isCheckScroll = true
var isCheckRevHex = false
var isCheckSendHex = false
var sendData = ""
var fall_flag = 0
var led_flag = 0
var rect_flag = 0
var tri_flag = 0
var left_flag = 0
var right_flag = 0

Page({
  /**
   * 页面的初始数据
   */
  data: {
    textRevData: "",
    scrollIntoView: "scroll-view-bottom",
    transform: "",
    color_fall: "green",
    color_l: "green",
    color_r: "green",
    color_led: "green",
    color_rec: "green",
    color_tri: "green"
  },
  btnFallHandle(e){
    fall_flag++;
    if(fall_flag>100)
        fall_flag=0;
    if(fall_flag%2==1)
    {
        console.log("fall_on");
        ecBLE.easySendData("1b", true);
        this.setData({
            color_fall: "blue"
        })
    }
    else
    {
        console.log("fall_off");
        ecBLE.easySendData("1c", true);
        this.setData({
            color_fall: "green"
        })
    }
  },
  btnLeftHandle(e){
    left_flag++;
    if(left_flag>100)
        left_flag=0;
    if(left_flag%2==1)
    {
        console.log("left_on");
        ecBLE.easySendData("11", true);
        this.setData({
            color_l: "red"
        })
    }
    else
    {
        console.log("left_off");
        ecBLE.easySendData("12", true);
        this.setData({
            color_l: "green"
        })
    } 
  },
  btnRightHandle(e){
    right_flag++;
    if(right_flag>100)
        right_flag=0;
    if(right_flag%2==1)
    {
        console.log("right_on");
        ecBLE.easySendData("13", true);
        this.setData({
            color_r: "red"
        })
    }
    else
    {
        console.log("right_off");
        ecBLE.easySendData("14", true);
        this.setData({
            color_r: "green"
        })
    }
  },
  btnLedHandle(e){
    led_flag++;
    if(led_flag>100)
        led_flag=0;
    if(led_flag%2==1)
    {
        console.log("led_on");
        ecBLE.easySendData("15", true);
        this.setData({
            color_led: "red"
        })
    }
    else
    {
        console.log("led_off");
        ecBLE.easySendData("16", true);
        this.setData({
            color_led: "green"
        })
    }
  },
  btnRectHandle(e){
    rect_flag++;
    if(rect_flag>100)
        rect_flag=0;
    if(rect_flag%2==1)
    {
        console.log("rect_on");
        ecBLE.easySendData("17", true);
        this.setData({
            color_rec: "red"
        })
    }
    else
    {
        console.log("rect_off");
        ecBLE.easySendData("18", true);
        this.setData({
            color_rec: "green"
        })
    }
  },
  btnTriHandle(e){
    tri_flag++;
    if(tri_flag>100)
        tri_flag=0;
    if(tri_flag%2==1)
    {
        console.log("tri_on");
        ecBLE.easySendData("19", true);
        this.setData({
            color_tri: "red"
        })
    }
    else
    {
        console.log("tri_off");
        ecBLE.easySendData("1a", true);
        this.setData({
            color_tri: "green"
        })
    }
  },
  
  startPoint: "",

  startFn(e) {
    let touch = e.touches[0]
    this.startPoint = [touch.pageX, touch.pageY] //横坐标和纵坐标
    console.log(touch.pageX,touch.pageY)
  },
  moveFn(e) {
    let touch = e.touches[0]
    let point = [touch.pageX, touch.pageY]
    //console.log(touch.pageX,touch.pageY)
    let differ = [point[0] - this.startPoint[0], point[1] - this.startPoint[1]]
    let distance = Math.sqrt(differ[0] * differ[0] + differ[1] * differ[1])
    if (distance > MIN_DISTANCE) {
      // 摇杆始终切到边缘，实际位移距离无关紧要，只关心移动方向
      let rate = RADIUS / distance
      let position = [differ[0] * rate*1.3, differ[1] * rate*1.3]
      this.setData({
        transform: "transform: translate(" + position[0] + "px, " + position[1] + "px)"
      })
       // console.log(e.touches[0].pageX,e.touches[0].pageY)
        //console.log(distance);
    if(e.touches[0].pageX<270 && e.touches[0].pageX>130 && e.touches[0].pageY < 170) {
         console.log("F");
        ecBLE.easySendData("01", true);
    }
    else if(e.touches[0].pageX<170 && e.touches[0].pageY>130 && e.touches[0].pageY < 270) {
         console.log("L");
        ecBLE.easySendData("04", true);
    }
    else if(e.touches[0].pageX>230 && e.touches[0].pageY>130 && e.touches[0].pageY < 270) {
         console.log("R");
        ecBLE.easySendData("02", true);
    }
    else if(e.touches[0].pageX<270 && e.touches[0].pageX>130 && e.touches[0].pageY > 230) {
         console.log("B");
        ecBLE.easySendData("03", true);
    }
    else{
        console.log(0);
        ecBLE.easySendData("00", true);
    }
     }
  },
  endFn(e) {
    this.setData({
      transform: ""
    });
    ecBLE.easySendData("00", true);
  },

  sliderChange:function(e){
      console.log(e.detail.value);
        if(e.detail.value==0)
        ecBLE.easySendData("88",true);
        else if(e.detail.value==1)
        ecBLE.easySendData("06",true);
        else if(e.detail.value==2)
        ecBLE.easySendData("07",true);
        else if(e.detail.value==3)
        ecBLE.easySendData("08",true);
 
  },

  /**
   * 生命周期函数--监听页面加载
   */
  onLoad: function (options) {
    const ctx = this
    //on disconnect
    ecBLE.onBLEConnectionStateChange(() => {
      ctx.showModal("提示", "设备断开连接")
    })
    //receive data
    ecBLE.onBLECharacteristicValueChange((str, strHex) => {
      let data = ctx.data.textRevData + "[" + ctx.dateFormat("hh:mm:ss,S", new Date()) + "]:" + (isCheckRevHex ? strHex : str) + "\r\n"
      console.log(data)
      ctx.setData({ textRevData: data })
      if (isCheckScroll) ctx.setData({ scrollIntoView: "scroll-view-bottom" })//scroll to bottom
    })
  },

    /**
     * 生命周期函数--监听页面初次渲染完成
     */
    onReady() {

    },

    /**
     * 生命周期函数--监听页面显示
     */
    onShow() {

    },

    /**
     * 生命周期函数--监听页面隐藏
     */
    onHide() {

    },

    /**
     * 生命周期函数--监听页面卸载
     */
    onUnload: function () {
        ecBLE.onBLEConnectionStateChange(() => { })
        ecBLE.closeBLEConnection()
    },

    /**
     * 页面相关事件处理函数--监听用户下拉动作
     */
    onPullDownRefresh() {

    },

    /**
     * 页面上拉触底事件的处理函数
     */
    onReachBottom() {

    },

    /**
     * 用户点击右上角分享
     */
    onShareAppMessage() {

    }
})