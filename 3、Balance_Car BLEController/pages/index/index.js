// index.js
// 获取应用实例
const app = getApp()
const regeneratorRuntime = require('../../utils/regenerator/runtime.js')
const ecBLE = require('../../utils/ecBLE.js')
 
Page({
    data: {
        deviceListData: [],
        deviceListDataShow: [],
    },
    async onLoad() {
        const ctx = this
        setInterval(() => {
            ctx.setData({ deviceListDataShow: ctx.data.deviceListData })
        }, 600)
    },
    async onShow() {
        const ctx = this
        this.setData({ deviceListData: [] })
        this.setData({ deviceListDataShow: [] })
        setTimeout(() => {
            ctx.startDiscovery()
        }, 100)
    },
    onPullDownRefresh: function () {
        this.setData({ deviceListData: [] })
        this.setData({ deviceListDataShow: [] })
        const ctx = this
        setTimeout(() => {
            wx.stopPullDownRefresh()
            ctx.startDiscovery()
        }, 500)
    },
    async listViewTap(event) {
        const ctx = this
        ctx.showLoading()
        // ecBLE.easyConnect(event.currentTarget.dataset.name, (res) => {
        //     ctx.hideLoading()
        //     if (res.ok) {
        //         //ctx.showModal("提示", "连接成功")
        //         ctx.gotoDevicePage()
        //     } else {
        //         ctx.showModal("提示", "连接失败,errCode=" + res.errCode + ",errMsg=" + res.errMsg)
        //     }
        // })

        let res = await ecBLE.easyConnect(event.currentTarget.dataset.name, () => { })
        ctx.hideLoading()
        if (res.ok) {
            //ctx.showModal("提示", "连接成功")
            ctx.gotoDevicePage()
        } else {
            ctx.showModal("提示", "连接失败,errCode=" + res.errCode + ",errMsg=" + res.errMsg)
        }
    },
    showModal(title, content) {
        wx.showModal({
            title: title,
            content: content,
            showCancel: false
        })
    },
    showLoading() {
        wx.showToast({
            title: '设备连接中',
            icon: 'loading',
            duration: 3600000,
            mask: true
        })
    },
    hideLoading() {
        wx.hideLoading()
    },
    gotoDevicePage() {
        ecBLE.stopBluetoothDevicesDiscovery()
        wx.navigateTo({
            url: '../test/test'
        })
    },
    async startDiscovery(){
        const ctx = this
        var res = await ecBLE.openBluetoothAdapter()
        if (res.ok) {
            console.log("start search")  
            ecBLE.startBluetoothDevicesDiscovery((name, rssi) => {
                // console.log("name:"+name+"|rssi:"+rssi)
                for (const item of ctx.data.deviceListData) {
                    if (item.name === name) {
                        item.rssi = rssi
                        return
                    }
                }
                ctx.data.deviceListData.push({ name, rssi })
            })
        }
        else {
            this.showModal("提示", "打开蓝牙适配器失败: " + res.errMsg)
        }
    },
})
