import { faEllipsis, faHashtag, faIdCard, faKey, faMask } from "@fortawesome/free-solid-svg-icons"
import IconAndTitle, { IconAndTitleProps } from "./IconAndTitle"
import { EBerytusChallengeType } from "@berytus/enums";

export interface ChallengePictureProps extends Omit<IconAndTitleProps, 'icon' | 'title'> {
    challengeType: typeof EBerytusChallengeType[keyof typeof EBerytusChallengeType];
    centered?: boolean
}

const challengeTypeIconTitleMap = {
    [EBerytusChallengeType.Identification]: {
        icon: faIdCard,
        title: "Identification Challenge"
    },
    [EBerytusChallengeType.Password]: {
        icon: faEllipsis,
        title: "Password Challenge"
    },
    [EBerytusChallengeType.SecureRemotePassword]: {
        icon: faMask,
        title: "SRP Challenge"
    },
    [EBerytusChallengeType.DigitalSignature]: {
        icon: faKey,
        title: "DigitalSignature Challenge"
    },
    [EBerytusChallengeType.OffChannelOtp]: {
        icon: faHashtag,
        title: "OTP Challenge"
    }
}

export default function ChallengePicture({ challengeType, ...otherProps }: ChallengePictureProps) {
    return (
        <IconAndTitle
            icon={challengeTypeIconTitleMap[challengeType].icon}
            title={challengeTypeIconTitleMap[challengeType].title}
            {...otherProps}
        />
    )
}