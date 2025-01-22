import { Button } from "@nextui-org/button";
import { Switch } from "@nextui-org/switch";
import { SetStateAction, useEffect, useState } from "react";
import { HexColorPicker } from "react-colorful";
import { Slider } from "@heroui/slider";

import { Select, SelectSection, SelectItem } from "@heroui/select";


export const rainbowColors = [
    { key: "red", label: "Red", color: "#FF1414" },
    { key: "orange", label: "Orange", color: "#FF7F00" },
    { key: "yellow", label: "Yellow", color: "#FFFF00" },
    { key: "green", label: "Green", color: "#00FF00" },
    { key: "blue", label: "Blue", color: "#0000FF" },
    { key: "indigo", label: "Indigo", color: "#4B0082" },
    { key: "violet", label: "Violet", color: "#8B00FF" },
];

function useDebounce(cb: number, delay: number) {
    const [debounceValue, setDebounceValue] = useState(cb);
    useEffect(() => {
        const handler = setTimeout(() => {
            setDebounceValue(cb);
        }, delay);

        return () => {
            clearTimeout(handler);
        };
    }, [cb, delay]);
    return debounceValue;
}


export default function ColorsPanel() {
    // 指针颜色
    const [southColor, setSouthColor] = useState("#ff1414");
    const [spawnColor, setSpawnColor] = useState("#ff1414");
    // 亮度
    const [brightness, setBrightness] = useState(56);

    const [debounceVal, setDebounceVal] = useState(56);

    const debounceTimeout = 200; // 200ms debounce

    const debounceValue = useDebounce(brightness, debounceTimeout);

    useEffect(() => {
        console.log("Debounced:", brightness);
        setDebounceVal(brightness);
        fetch(`/brightness?brightness=${encodeURIComponent(brightness)}`, {
            method: "POST",
        });
    }, [debounceValue]);

    useEffect(() => {
        fetch("/pointColors")
            .then(response => response.json())
            .then(data => {
                if (data.southColor && data.spawnColor) {
                    setSouthColor(data.ssid);
                    setSpawnColor(data.password);
                }
            });
        fetch("/brightness")
            .then(response => response.json())
            .then(data => {
                if (data.brightness) {
                    setBrightness(data.brightness);
                }
            });
        fetch(`/brightness`).then(response => response.json())
            .then(data => {
                if (data.brightness) {
                    setBrightness(data.brightness);
                }
            });
    }, [southColor, spawnColor, brightness]);


    const handleSliderChange = (value: number | number[]) => {
        if (!Array.isArray(value)) {
            setBrightness(value);
        }
    };

    function saveColors() {
        fetch(`/pointColors?southColor=${encodeURIComponent(southColor)}&spawnColor=${encodeURIComponent(spawnColor)}`, {
            method: "POST",
        });
    }

    return <div className="w-full flex flex-col items-center justify-center flex-wrap gap-4">
        <p className="px-3 text-start w-full">自定义亮度和指针颜色,<br /> 亮度实时生效, 颜色重启后生效</p>
        <Slider
            className="max-w-md"
            defaultValue={56}
            label="亮度"
            value={brightness}
            onChange={handleSliderChange} // 监听变化事件
            maxValue={100}
            minValue={0}
            step={1}
        />
        <Select
            className="max-w-xs"
            items={rainbowColors}
            label="指南针模式指针颜色"
            placeholder="请选择一种颜色"
        >
            {(color) => <SelectItem>{color.label}</SelectItem>}
        </Select>
        <Select
            className="max-w-xs"
            items={rainbowColors}
            label="出生针模式指针颜色"
            placeholder="请选择一种颜色"
        >
            {(color) => <SelectItem>{color.label}</SelectItem>}
        </Select>
        <Button color="primary" variant="ghost" className="max-w-xs w-full" onClick={saveColors}>
            保存
        </Button>
    </div>;
}