import { Button } from "@heroui/button";
import { useState } from "react";

export default function Reset() {

    const [color, setColor] = useState("primary");
    function onResetButtonClick() {
        console.log("Reset");
        fetch("/reset");
    }

    return <div className="w-full flex flex-col items-center justify-center flex-wrap gap-4">
        <p className="text-start w-full">Warning:<br /> This will reset all settings and reboot the device.<br />You will lose connection to the device.</p>
        <Button
            variant="ghost"
            className="w-full max-w-xs"
            color={color as "danger" | "primary"}
            onMouseEnter={() => setColor("danger")}
            onMouseLeave={() => setColor("primary")}
            onClick={onResetButtonClick}
        >
            Reset
        </Button>
    </div>;
}