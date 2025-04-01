import { Switch } from "@heroui/switch";
import { useState } from "react";

export default function DebugPanel() {

    const [fill, setFill] = useState(false);

    function onFillChange(value: boolean) {
        setFill(value);
    }

    function queryWifiList() {
        console.log("Query WiFi List");
        // 发起一个请求，获取WiFi列表
        // fetch("/get")
        //     .then(response => response.json())
        //     .then(data => {
        //         console.log(data);
        //     })
        //     .catch(error => {
        //         console.error("Error fetching WiFi list:", error);
        //     });
        // 把color post出去

    }

    return <div className="w-full flex flex-col flex-wrap gap-4">
        <Switch className="w-full text-start " checked={fill} onValueChange={onFillChange}>Should we arrive the nether?</Switch>
    </div>;
}