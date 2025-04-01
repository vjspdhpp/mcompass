'use client';
import {
  Navbar as NextUINavbar,
  NavbarContent,
  NavbarMenu,
  NavbarMenuToggle,
  NavbarBrand,
  NavbarItem,
  NavbarMenuItem,
} from "@heroui/navbar";
import { Modal, ModalBody, ModalContent, ModalFooter, ModalHeader, useDisclosure } from "@heroui/modal";

import { Button } from "@heroui/button";
import { Link } from "@heroui/link";
import { Input } from "@heroui/input";
import { link as linkStyles } from "@heroui/theme";
import NextLink from "next/link";
import clsx from "clsx";

import { siteConfig } from "@/config/site";
import { ThemeSwitch } from "@/components/theme-switch";
import {
  GithubIcon,
  HeartFilledIcon,
  Logo,
} from "@/components/icons";
import { Switch } from "@heroui/switch";
import { cn } from "@heroui/theme";
import { useEffect, useState } from "react";
import { Radio, RadioGroup } from "@heroui/radio";

export const Navbar = () => {
  const { isOpen, onOpen, onOpenChange } = useDisclosure();

  const [serverMode, setServerMode] = useState("0");

  const [deviceModel, setDeviceModel] = useState<string>("gps");

  // 当 Switch 状态变化时触发
  const handleSwitchChange = (value: boolean | ((prevState: boolean) => boolean)) => {
    setServerMode(value ? "1" : "0");
    console.log("Switch 当前状态:", value);
  };

  useEffect(() => {
    fetch("/adveancedConfig")
      .then(response => response.json())
      .then(data => {
        setServerMode(data.serverMode);
        setDeviceModel(data.model == "0" ? "lite" : "gps");
      });
  }, [])

  // 保存实验特性配置
  function saveAdvancedConfig() {
    const params = new URLSearchParams({
      serverMode: serverMode,
      model: deviceModel == "lite" ? "0" : "1",
    });

    fetch(`/adveancedConfig?${params.toString()}`, {
      method: "POST",
    });
  }

  return (
    <NextUINavbar maxWidth="xl" position="sticky">
      <NavbarContent className="basis-1/5 sm:basis-full" justify="start">
        <NavbarBrand as="li" className="gap-3 max-w-fit">
          <NextLink className="flex justify-start items-center gap-1" href="/">
            <Logo />
            <p className="font-bold text-inherit">Compass</p>
          </NextLink>
        </NavbarBrand>
        <ul className="hidden lg:flex gap-4 justify-start ml-2">
          {siteConfig.navItems.map((item) => (
            <NavbarItem key={item.href}>
              <NextLink
                className={clsx(
                  linkStyles({ color: "foreground" }),
                  "data-[active=true]:text-primary data-[active=true]:font-medium",
                )}
                color="foreground"
                href={item.href}
              >
                {item.label}
              </NextLink>
            </NavbarItem>
          ))}
        </ul>
      </NavbarContent>

      <NavbarContent
        className="hidden sm:flex basis-1/5 sm:basis-full"
        justify="end"
      >
        <NavbarItem className="hidden sm:flex gap-2">
          <Link isExternal aria-label="Github" href={siteConfig.links.github}>
            <GithubIcon className="text-default-500" />
          </Link>
          <ThemeSwitch />
        </NavbarItem>
      </NavbarContent>

      <NavbarContent className="sm:hidden basis-1 pl-4" justify="end">
        <Link isExternal aria-label="Github" href={siteConfig.links.github}>
          <GithubIcon className="text-default-500" />
        </Link>
        <ThemeSwitch />
      </NavbarContent>
      <NavbarItem>
        <Button
          className="text-sm font-normal text-default-600 bg-default-100"
          startContent={<HeartFilledIcon className="text-danger" />}
          variant="flat"
          onPress={onOpen}
        >
          实验特性
        </Button>
        <Modal isOpen={isOpen} onOpenChange={onOpenChange}>
          <ModalContent>
            {(onClose) => (
              <>
                <ModalHeader className="flex flex-col gap-1">实验特性</ModalHeader>
                <ModalBody>
                  <p>
                    谨慎启用以下选项，重启后生效
                  </p>
                  <Switch
                    isSelected={serverMode == "1"} // 绑定状态
                    onValueChange={handleSwitchChange} // 状态变化时的回调
                    classNames={{
                      base: cn(
                        "inline-flex flex-row-reverse w-full max-w-md bg-content1 hover:bg-content2 items-center",
                        "justify-between cursor-pointer rounded-lg gap-2 p-4 border-2 border-transparent",
                        "data-[selected=true]:border-primary",
                      ),
                      wrapper: "p-0 h-4 overflow-visible",
                      thumb: cn(
                        "w-6 h-6 border-2 shadow-lg",
                        "group-data-[hover=true]:border-primary",
                        //selected
                        "group-data-[selected=true]:ms-6",
                        // pressed
                        "group-data-[pressed=true]:w-7",
                        "group-data-[selected]:group-data-[pressed]:ms-4",
                      ),
                    }}
                  >
                    <div className="flex flex-col gap-1">
                      <p className="text-medium">使用蓝牙进行配置</p>
                      <p className="text-tiny text-default-400">
                        蓝牙配置需要对应的程序配合
                      </p>
                    </div>
                  </Switch>
                  <RadioGroup label="修改设备类型" orientation="horizontal" value={deviceModel} onValueChange={setDeviceModel}>
                    <Radio value="lite">标准版</Radio>
                    <Radio value="gps">GPS版</Radio>
                  </RadioGroup>
                </ModalBody>
                <ModalFooter>
                  <Button color="danger" variant="light" onPress={onClose}>
                    关闭
                  </Button>
                  <Button color="primary" onPress={() => {
                    saveAdvancedConfig();
                    onClose();
                  }}>
                    保存
                  </Button>
                </ModalFooter>
              </>
            )}
          </ModalContent>
        </Modal>
      </NavbarItem>
    </NextUINavbar>
  );
};
