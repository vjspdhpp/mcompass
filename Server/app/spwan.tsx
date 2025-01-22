import { useEffect, useState } from "react";
import { Button } from "@nextui-org/button";
import { Input } from "@nextui-org/input";
import { Popover, PopoverTrigger, PopoverContent } from "@nextui-org/popover";
import { Progress } from "@nextui-org/progress";

export default function SpawnPanel() {
    const [latitude, setLatitude] = useState('');
    const [longitude, setLongitude] = useState('');

    const [canSave, setCanSave] = useState(false);
    const [isSaving, setIsSaving] = useState(false);
    const [errorPopover, setErrorPopover] = useState(false);

    useEffect(() => {
        setIsSaving(true);
        // 获取当前的经纬度
        fetch("/spawn")
            .then(response => response.json())
            .then(data => {
                if (data.latitude && data.longitude) {
                    setLatitude(data.latitude);
                    setLongitude(data.longitude);
                    setCanSave(true);
                }
            }).finally(() => {
                setIsSaving(false);
            });
    }, []);

    function onLatitudeChange(e: React.ChangeEvent<HTMLInputElement>) {
        const lat = e.target.value;
        setLatitude(lat);
        setCanSave(lat !== '' && longitude !== '');
        setErrorPopover(false);
    }

    function onLongitudeChange(e: React.ChangeEvent<HTMLInputElement>) {
        const lon = e.target.value;
        setLongitude(lon);
        setCanSave(latitude !== '' && lon !== '');
        setErrorPopover(false);
    }

    function handleSave() {
        const lat = parseFloat(latitude);
        const lon = parseFloat(longitude);
        // make sure the lat is in the range of -90 to 90 
        if (lat && lon && lat >= -90 && lat <= 90 && lon >= -180 && lon <= 180) {
            console.log("Save ", lat, lon);
            setErrorPopover(false);
            setIsSaving(true);
            fetch(`/spawn?latitude=${latitude}&longitude=${longitude}`, {
                method: "POST"
            }).finally(() => {
                setIsSaving(false);
            });
        } else {
            console.log("Please enter both latitude and longitude");
            // show a toast
            setErrorPopover(true);
            setTimeout(() => {
                setErrorPopover(false);
            }, 2000);
        }
    }
    return <div className="w-full flex flex-col items-center justify-center flex-wrap gap-4">
        <p className="px-3 text-start w-full">设置出生点<br />有GPS信号时候会指向这个地点</p>
        <Input type="number" label="纬度" value={latitude} onChange={onLatitudeChange} />
        <Input type="number" label="经度" value={longitude} onChange={onLongitudeChange} />
        <Popover placement="bottom" isOpen={errorPopover} >
            <PopoverTrigger>
                <Button color="primary" variant="ghost" className="max-w-xs w-full" onClick={handleSave} isDisabled={!canSave}>
                    {isSaving ? <Progress
                        size="sm"
                        isIndeterminate
                        aria-label="Loading..."
                        className="max-w-md"
                    /> : "保存"}
                </Button>
            </PopoverTrigger>
            <PopoverContent>
                <div className="px-1 py-2">
                    <div className="text-small font-bold">Value Error</div>
                    <div className="text-tiny">纬度值范围应该是-90~90,经度值范围应该是-180~180.</div>
                </div>
            </PopoverContent>
        </Popover>
    </div>;
}