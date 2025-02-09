import { Button } from "@nextui-org/button";
import { useEffect, useState } from "react";

export default function InfoPanel() {
    const [deviceInfo, setDeviceInfo] = useState({
        buildDate: "未知",
        buildTime: "未知",
        buildVersion: "未知",
        gitBranch: "未知",
        gitCommit: "未知",
        gpsStatus: "0",
        sensorStatus: "0",
    });

    useEffect(() => {
        fetch("/info")
            .then(response => response.json())
            .then(data => {
                if (data) {
                    setDeviceInfo(data);
                }
            });
    }, [])

    function reboot() {
        fetch("/reboot");
    }

    return <div>
        <ul>
            <li>固件版本: {deviceInfo.buildVersion}</li>
            <li>固件分支: {deviceInfo.gitBranch}</li>
            <li>提交ID: {deviceInfo.gitCommit}</li>
            <li>构建时间: {deviceInfo.buildTime}</li>
            <li>构建日期: {deviceInfo.buildDate}</li>
            <li>GPS状态: {deviceInfo.gpsStatus === "1" ? "可用" : "不可用"}</li>
            <li>地磁传感器状态: {deviceInfo.sensorStatus === "1" ? "可用" : "不可用"}</li>
            <Button color="danger" className="max-w-xs w-full mt-4" onClick={reboot}>
                重启
            </Button>
        </ul>
    </div>;
}